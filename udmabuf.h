#pragma once

#include <string>
#include <string_view>
#include <fstream>

class udmabuf
{
public:
    udmabuf();
    explicit udmabuf(std::string_view name, bool sync = false);

    ~udmabuf() noexcept;

    udmabuf(udmabuf&& other);
    udmabuf(const udmabuf&) = delete;
    udmabuf& operator=(udmabuf&& other);
    void operator=(const udmabuf&) = delete;

    std::ifstream get_prop(std::string_view name) const
    {
        std::string path = (m_class_path + "/").append(name);
        return std::ifstream(path);
    }

    std::ofstream set_prop(std::string_view name) const
    {
        std::string path = (m_class_path + "/").append(name);
        return std::ofstream(path);
    }

    void sync_for_cpu(unsigned long offset, unsigned long length) const noexcept;

    const void *get() const noexcept;
    void* get() noexcept;
    size_t size() const noexcept;
    size_t phys_addr() const noexcept;
    const std::string &name() const noexcept;

private:
    void map(int o_sync);
    void swap(udmabuf& other);

private:
    int m_fd = -1;
    std::string m_name;
    std::string m_dev_name;
    std::string m_class_path;
    size_t      m_phys_addr = 0;
    size_t      m_size = 0;
    void*       m_ptr = nullptr;
};

