#include <fcntl.h>
#include <sys/mman.h>
#include <syslog.h>
#include <unistd.h>
#include <cstring>
#include "conn.h"

bool Conn::open(int c_id, bool create) {
    (void)create;
    id = c_id;
    name = "/shm" + std::to_string(c_id);
    int fd = shm_open(name.c_str(), O_RDWR | O_CREAT, 0666);
    if (fd != -1) {
        ftruncate(fd, sizeof(int));
        f_descr = static_cast<int*>(
                    mmap(nullptr, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        if (f_descr == MAP_FAILED) {
            syslog(LOG_ERR, "Mmap failed");
            return false;
        }
        syslog(LOG_INFO, "SHM connection is set");
        return true;
    }
    syslog(LOG_ERR, "SHM: file descriptor is -1");
    return false;
}

bool Conn::read(void* buf, size_t count) {
    return memcpy(buf, f_descr, count) != nullptr;
}

bool Conn::write(void* buf, size_t count) {
    return memcpy(f_descr, buf, count) != nullptr;
}

bool Conn::close() {
    munmap(f_descr, sizeof(int));
    shm_unlink(name.c_str());
    syslog(LOG_INFO, "SHM connection is closed");
    return true;
}
