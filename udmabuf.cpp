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

const char* dev_prefix = "/dev/";
const char* sys_prefix = "/sys/class/u-dma-buf/";

}

udmabuf::udmabuf() = default;

udmabuf::udmabuf(std::string_view name)
    : m_name(name),
      m_dev_name(dev_prefix),
      m_class_path(sys_prefix)
{
    m_dev_name.append(name);
    m_class_path.append(name);

    get_prop("phys_addr") >> std::hex >> m_phys_addr;
    get_prop("size") >> std::dec >> m_size;

    std::cout << fmt::format("addr: 0x{0:0{1}X}, size: {2}", m_phys_addr, sizeof(m_phys_addr)*2, m_size) << '\n';

    map();
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
    const unsigned long  sync_offset = offset;
    const unsigned long  sync_size = length;
    unsigned int   sync_direction = 1;
    unsigned long  sync_for_cpu   = 1;
    auto path = m_class_path + "/sync_for_cpu";
    if (int fd; (fd  = ::open(path.c_str(), O_WRONLY)) != -1) {
        ::snprintf(attr, sizeof(attr),
                   "0x%08X%08X",
                   (uint32_t(sync_offset) & 0xFFFFFFFF),
                   (uint32_t(sync_size) & 0xFFFFFFF0) | (uint32_t(sync_direction) << 2) | uint32_t(sync_for_cpu));
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

size_t udmabuf::phys_addr() const noexcept
{
    return m_phys_addr;
}

void udmabuf::map()
{
    // TBD: optional O_SYNC
    if (int fd; (fd = open(m_dev_name.c_str(), O_RDWR)) != -1) {
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
    std::swap(m_class_path, other.m_class_path);
    std::swap(m_phys_addr, other.m_phys_addr);
    std::swap(m_size, other.m_size);
    std::swap(m_ptr, other.m_ptr);
}
