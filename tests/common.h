#pragma once

#define BUF_NAME      "udmabuf0"
#define BUF_SIZE      (2*1024*1024)
#define BUF_PHYS_ADDR (0xDEADBEEF)

extern void moc_set_sysroot(const char* sysroot) noexcept;
extern const char* moc_get_sysroot() noexcept;

