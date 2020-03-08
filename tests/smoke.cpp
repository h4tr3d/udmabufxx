#include <type_traits>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <fmt/format.h>

#include "udmabuf.h"
#include "catch2/catch.hpp"

#include "common.h"

namespace fs = std::filesystem;

TEST_CASE("Creation instance", "[smoke]")
{
    udmabuf buf{BUF_NAME};

    REQUIRE(buf.name() == BUF_NAME);
    REQUIRE(buf.get() != nullptr);
    REQUIRE(buf.phys_addr() == BUF_PHYS_ADDR);
    REQUIRE(buf.size() == BUF_SIZE);
}

TEST_CASE("Empty creation", "[smoke]")
{
    udmabuf buf;

    REQUIRE(buf.name() == "");
    REQUIRE(buf.get() == nullptr);
    REQUIRE(buf.phys_addr() == 0);
    REQUIRE(buf.size() == 0);
}

TEST_CASE("Copy & Move", "[smoke]")
{
    REQUIRE(std::is_copy_assignable_v<udmabuf> == false);
    REQUIRE(std::is_copy_constructible_v<udmabuf> == false);
    REQUIRE(std::is_move_assignable_v<udmabuf> == true);
    REQUIRE(std::is_move_constructible_v<udmabuf> == true);

    {
        udmabuf buf0;
        udmabuf buf1{BUF_NAME};

        std::swap(buf0, buf1);

        REQUIRE(buf0.name() == BUF_NAME);
        REQUIRE(buf0.get() != nullptr);
        REQUIRE(buf0.phys_addr() == BUF_PHYS_ADDR);
        REQUIRE(buf0.size() == BUF_SIZE);

        REQUIRE(buf1.name() == "");
        REQUIRE(buf1.get() == nullptr);
        REQUIRE(buf1.phys_addr() == 0);
        REQUIRE(buf1.size() == 0);
    }

    {
        udmabuf buf0;
        udmabuf buf1{BUF_NAME};

        buf0 = std::move(buf1);

        REQUIRE(buf0.name() == BUF_NAME);
        REQUIRE(buf0.get() != nullptr);
        REQUIRE(buf0.phys_addr() == BUF_PHYS_ADDR);
        REQUIRE(buf0.size() == BUF_SIZE);

        REQUIRE(buf1.name() == "");
        REQUIRE(buf1.get() == nullptr);
        REQUIRE(buf1.phys_addr() == 0);
        REQUIRE(buf1.size() == 0);
    }

    {
        udmabuf buf0;

        buf0 = udmabuf{BUF_NAME};

        REQUIRE(buf0.name() == BUF_NAME);
        REQUIRE(buf0.get() != nullptr);
        REQUIRE(buf0.phys_addr() == BUF_PHYS_ADDR);
        REQUIRE(buf0.size() == BUF_SIZE);
    }

    {
        udmabuf buf0{BUF_NAME};
        udmabuf buf1{std::move(buf0)};

        REQUIRE(buf1.name() == BUF_NAME);
        REQUIRE(buf1.get() != nullptr);
        REQUIRE(buf1.phys_addr() == BUF_PHYS_ADDR);
        REQUIRE(buf1.size() == BUF_SIZE);

        REQUIRE(buf0.name() == "");
        REQUIRE(buf0.get() == nullptr);
        REQUIRE(buf0.phys_addr() == 0);
        REQUIRE(buf0.size() == 0);
    }
}

TEST_CASE("Sync for CPU", "[smoke]")
{
    udmabuf buf{BUF_NAME};
    constexpr static uint32_t sync_direction = 1;
    constexpr static uint32_t sync_for_cpu   = 1;

    struct cases_t {
        size_t offset;
        size_t size;
    } cases[] = {
        {0, buf.size()},
        {1, buf.size() - 1},
        {4096, 4096}
    };

    // just assumption for tests
    static_assert (BUF_SIZE > 2*4096);

    auto sysroot = moc_get_sysroot();
    fs::path class_path = fs::path(sysroot) / "sys/class/u-dma-buf" / BUF_NAME;

    for (auto const& v : cases) {
        buf.sync_for_cpu(v.offset, v.size);

        std::clog << "sync: " << v.offset << " / " << v.size << '\n';

        std::ifstream ifs(class_path / "sync_for_cpu", std::ios_base::out|std::ios_base::binary);
        std::string value;
        std::string expected = fmt::format("0x{:08X}{:08X}",
                                           v.offset,
                                           (uint32_t(v.size) & 0xFFFFFFF0) | sync_direction<<2 | sync_for_cpu<<0);

        ifs >> value;

        REQUIRE(value == expected);
    }
}

TEST_CASE("Sync for Device", "[smoke]")
{
    udmabuf buf{BUF_NAME};
    constexpr static uint32_t sync_direction = 1;
    constexpr static uint32_t sync_for_cpu   = 1;

    struct cases_t {
        size_t offset;
        size_t size;
    } cases[] = {
        {0, buf.size()},
        {1, buf.size() - 1},
        {4096, 4096}
    };

    // just assumption for tests
    static_assert (BUF_SIZE > 2*4096);

    auto sysroot = moc_get_sysroot();
    fs::path class_path = fs::path(sysroot) / "sys/class/u-dma-buf" / BUF_NAME;

    for (auto const& v : cases) {
        buf.sync_for_device(v.offset, v.size);

        std::clog << "sync: " << v.offset << " / " << v.size << '\n';

        std::ifstream ifs(class_path / "sync_for_device", std::ios_base::out|std::ios_base::binary);
        std::string value;
        std::string expected = fmt::format("0x{:08X}{:08X}",
                                           v.offset,
                                           (uint32_t(v.size) & 0xFFFFFFF0) | sync_direction<<2 | sync_for_cpu<<0);

        ifs >> value;

        REQUIRE(value == expected);
    }
}

TEST_CASE("DMA Coherent check", "[smoke]")
{
    udmabuf buf{BUF_NAME};
    REQUIRE(buf.is_dma_coherent());
}

