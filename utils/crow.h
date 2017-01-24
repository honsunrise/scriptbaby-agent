//
// Created by zhsyourai on 1/10/17.
//

#ifndef PONGO_AGENT_CROW_H
#define PONGO_AGENT_CROW_H
#include <stdio.h>
#include <stdint.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>


class crow {
public:
    crow();

    virtual ~crow();

    void collection();
private:
    // Percent
    double mem_percent;
    double cpu_percent;
    double disk_percent;
    // Value mem
    uint64_t virtual_mem_total;
    uint64_t virtual_mem_used;
    uint64_t phys_mem_total;
    uint64_t phys_mem_used;
    // Value cpu
    uint64_t cpu_last_total_user;
    uint64_t cpu_total_user;
    uint64_t cpu_last_total_user_low;
    uint64_t cpu_total_user_low;
    uint64_t cpu_last_total_sys;
    uint64_t cpu_total_sys;
    uint64_t cpu_last_total_idle;
    uint64_t cpu_total_idle;
    // Value disk
    uint64_t disk_total;
    uint64_t disk_used;
    uint64_t disk_free;

    void _init();
    void _c_memory();
    void _c_cpu();
    void _c_disk();
};


#endif //PONGO_AGENT_CROW_H
