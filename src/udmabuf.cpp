#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstdlib>
#include <cstring>

#include <fstream>
#include <iostream>

#include <fmt/format.h>

#include "udmabuf.h"

// https://github.com/ikwzm/udmabuf/issues/38
//#define REAL_PAGE_ALLOCATION_WA

namespace {

const char* dev_prefix = "{}/dev/{}";

#define SYS_PREFIX "{}/sys/class/u-dma-buf/{}/"

const char* param_phys_addr       = SYS_PREFIX "phys_addr";
const char* param_size            = SYS_PREFIX "size";
//const char* param_sync_mode     = SYS_PREFIX "sync_mode";
//const char* param_debug_vma     = SYS_PREFIX "debug_vma";
const char* param_sync_for_cpu    = SYS_PREFIX "sync_for_cpu";
const char* param_sync_for_device = SYS_PREFIX "sync_for_device";

const char* moc_sysroot = "";

}

namespace {
std::ifstream get_prop(std::string_view prop, std::string_view dma_name)
{
    return std::ifstream(fmt::format(prop, moc_sysroot, dma_name));
}

[[maybe_unused]]
std::ofstream set_prop(std::string_view prop, std::string_view dma_name)
{
    return std::ofstream(fmt::format(prop, moc_sysroot, dma_name));
}
}

#ifdef TESTS_MOC
void moc_set_sysroot(const char* sysroot) noexcept
{
    moc_sysroot = sysroot;
}
const char* moc_get_sysroot() noexcept
{
    return moc_sysroot;
}
#endif

udmabuf::udmabuf() = default;

udmabuf::udmabuf(std::string_view name, bool sync)
    : m_name(name),
      m_dev_name(fmt::format(dev_prefix, moc_sysroot, name))
{
    get_prop(param_phys_addr, m_name) >> std::hex >> m_phys_addr;
    get_prop(param_size, m_name) >> std::dec >> m_size;

    std::cout << fmt::format("addr: 0x{0:0{1}X}, size: {2}", m_phys_addr, sizeof(m_phys_addr)*2, m_size) << '\n';

    map(sync ? O_SYNC : 0);
}

udmabuf::~udmabuf() noexcept
{
    if (m_ptr && m_ptr != MAP_FAILED) {
        ::munmap(m_ptr, m_size);
    }
    ::close(m_fd);
}

udmabuf::udmabuf(udmabuf &&other)
    : udmabuf()
{
    swap(other);
}

udmabuf &udmabuf::operator=(udmabuf &&other)
{
    udmabuf(std::move(other)).swap(*this);
    return *this;
}

void udmabuf::sync_for_cpu(unsigned long offset, unsigned long length) const noexcept
{
    char attr[64];
    const unsigned long  sync_offset    = offset;
    const unsigned long  sync_size      = length;
    const unsigned int   sync_direction = 1;
    const unsigned long  sync_for_cpu   = 1;
    auto path = fmt::format(param_sync_for_cpu, moc_sysroot, m_name);
    if (int fd; (fd  = ::open(path.c_str(), O_WRONLY)) != -1) {
        ::snprintf(attr, sizeof(attr),
                   "0x%08X%08X",
                   (uint32_t(sync_offset) & 0xFFFFFFFF),
                   (uint32_t(sync_size) & 0xFFFFFFF0) | (uint32_t(sync_direction) << 2) | uint32_t(sync_for_cpu));
        ::write(fd, attr, strlen(attr));
        ::close(fd);
    }
}

void udmabuf::sync_for_device(unsigned long offset, unsigned long length) const noexcept
{
    char attr[64];
    const unsigned long  sync_offset     = offset;
    const unsigned long  sync_size       = length;
    const unsigned int   sync_direction  = 1;
    const unsigned long  sync_for_device = 1;
    auto path = fmt::format(param_sync_for_device, moc_sysroot, m_name);
    if (int fd; (fd  = open(path.c_str(), O_WRONLY)) != -1) {
        ::snprintf(attr, sizeof(attr),
                   "0x%08X%08X",
                   (uint32_t(sync_offset) & 0xFFFFFFFF),
                   (uint32_t(sync_size) & 0xFFFFFFF0) | (uint32_t(sync_direction) << 2) | uint32_t(sync_for_device));
        ::write(fd, attr, strlen(attr));
        ::close(fd);
    }
}

const void *udmabuf::get() const noexcept
{
    return m_ptr;
}

void *udmabuf::get() noexcept
{
    return m_ptr;
}

size_t udmabuf::size() const noexcept
{
    return m_size;
}

const std::string& udmabuf::name() const noexcept
{
    return m_name;
}

const std::string &udmabuf::dev_name() const noexcept
{
    return m_dev_name;
}

size_t udmabuf::phys_addr() const noexcept
{
    return m_phys_addr;
}

void udmabuf::map(int o_sync)
{
    if (int fd; (fd = open(m_dev_name.c_str(), O_RDWR | o_sync)) != -1) {
        m_ptr = mmap(nullptr, m_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        m_fd = fd;
        if (m_ptr == MAP_FAILED) {
            std::error_code ec{errno, std::system_category()};
            throw std::system_error(ec, fmt::format("Can't map CMA memory"));
        }
        fmt::print("cma vaddr: {0}\n", m_ptr);

#ifdef REAL_PAGE_ALLOCATION_WA
        // Write once to allocate physical memory to u-dma-buf virtual space.
        // Note: Do not use memset() for this.
        //       Because it does not work as expected.
        {
            auto   word_ptr = reinterpret_cast<uintptr_t*>(m_ptr);
            size_t words    = m_size / sizeof(uintptr_t);
            for (size_t i = 0 ; i < words; i++) {
                word_ptr[i] = 0;
            }
        }
#endif
    }
}

void udmabuf::swap(udmabuf &other)
{
    std::swap(m_fd, other.m_fd);
    std::swap(m_name, other.m_name);
    std::swap(m_dev_name, other.m_dev_name);
    std::swap(m_phys_addr, other.m_phys_addr);
    std::swap(m_size, other.m_size);
    std::swap(m_ptr, other.m_ptr);
}
