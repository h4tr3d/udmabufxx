#include <filesystem>
#include <fstream>
#include <fmt/format.h>

// In a Catch project with multiple files, dedicate one file to compile the
// source code of Catch itself and reuse the resulting object file for linking.

// Let Catch provide main():
//#define CATCH_CONFIG_MAIN
// Let Catch use provided main():
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

// That's it

// Compile implementation of Catch for use with files that do contain tests:
// - g++ -std=c++11 -Wall -I$(CATCH_SINGLE_INCLUDE) -c 000-CatchMain.cpp
// - cl -EHsc -I%CATCH_SINGLE_INCLUDE% -c 000-CatchMain.cpp

#include "common.h"

using namespace std;
namespace fs = std::filesystem;

struct context
{
    fs::path sysroot;
    const     string name = BUF_NAME;
    constexpr static size_t size = BUF_SIZE;

    fs::path device;
    fs::path sysprefix;

    context()
    {
        auto pwd = fs::current_path();
        std::clog << pwd << std::endl;

        sysroot = pwd / "sysroot";
        std::clog << sysroot << std::endl;

        // Configure MOC
        moc_set_sysroot(sysroot.c_str());

        // Directory structure
        device = sysroot / "dev" / name;
        sysprefix = sysroot / "sys/class/u-dma-buf" / name;
        fs::create_directories(sysroot / "dev");
        fs::create_directories(sysprefix);

        // Dev file itself
        {
            std::vector<char> buf;
            buf.resize(size);
            ofstream ofs(device, ios_base::out|ios_base::binary);
            ofs.write(buf.data(), buf.size());
        }

        // Parameters
        {
            ofstream ofs(sysprefix / "size", ios_base::out|ios_base::binary);
            ofs << fmt::format("{}", size);
        }
        {
            ofstream ofs(sysprefix / "phys_addr", ios_base::out|ios_base::binary);
            ofs << fmt::format("0x{:x}", BUF_PHYS_ADDR);
        }
        {
            ofstream ofs(sysprefix / "sync_for_cpu", ios_base::out|ios_base::binary);
            //ofs << fmt::format("0x{:x}", 0xDEADBEEF);
        }
    }

    ~context()
    {
        //fs::remove_all(sysroot / "dev");
        //fs::remove_all(sysroot / "sys/class/u-dma-buf" / name);
    }
};

int main(int argc, char** argv)
{
    context ctx;
    // Global setup...

    auto result = Catch::Session().run(argc, argv);

    // Globl clean up...

    return result;
}
