//
// Created by zhsyourai on 1/10/17.
//


#include <algorithm>
#include "crow.h"

crow::crow():mem_percent(0.0), cpu_percent(0.0), disk_percent(0.0) {
    _init();
}

crow::~crow() {

}

void crow::collection() {
    _c_cpu();
    _c_memory();
    _c_disk();
}

void crow::_init() {
    FILE* file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %lu %lu %lu %lu", &cpu_last_total_user, &cpu_last_total_user_low, &cpu_last_total_sys, &cpu_last_total_idle);
    fclose(file);
}

void crow::_c_memory() {
    struct sysinfo memInfo;
    sysinfo(&memInfo);
    virtual_mem_total = memInfo.totalram;
    virtual_mem_total += memInfo.totalswap;
    virtual_mem_total *= memInfo.mem_unit;

    virtual_mem_used = memInfo.totalram - memInfo.freeram;
    virtual_mem_used += memInfo.totalswap - memInfo.freeswap;
    virtual_mem_used *= memInfo.mem_unit;

    phys_mem_total = memInfo.totalram;
    phys_mem_total *= memInfo.mem_unit;

    phys_mem_used = memInfo.totalram - memInfo.freeram;
    phys_mem_used *= memInfo.mem_unit;

    mem_percent = ((double)phys_mem_used / phys_mem_total) * 100;
}

void crow::_c_cpu() {
    FILE* file;
    file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %lu %lu %lu %lu", &cpu_total_user, &cpu_total_user_low, &cpu_total_sys, &cpu_total_idle);
    fclose(file);

    if (cpu_total_user < cpu_last_total_user || cpu_total_user_low < cpu_last_total_user_low ||
            cpu_total_sys < cpu_last_total_sys || cpu_total_idle < cpu_last_total_idle){
        cpu_percent = -1.0;
    }
    else{
        uint64_t total = (cpu_total_user - cpu_last_total_user) +
                (cpu_total_user_low - cpu_last_total_user_low) + (cpu_total_sys - cpu_last_total_sys);
        cpu_percent = total;
        total += (cpu_total_idle - cpu_last_total_idle);
        cpu_percent /= std::max(total, (uint64_t)1);
        cpu_percent *= 100;
    }

    cpu_last_total_user = cpu_total_user;
    cpu_last_total_user_low = cpu_total_user_low;
    cpu_last_total_sys = cpu_total_sys;
    cpu_last_total_idle = cpu_total_idle;
}

void crow::_c_disk() {
    struct statvfs buf;
    if (!statvfs("/", &buf)) {
        uint64_t blksize, blocks, freeblks;

        blksize = buf.f_bsize;
        blocks = buf.f_blocks;
        freeblks = buf.f_bfree;

        disk_total = blocks * blksize;
        disk_free = freeblks * blksize;
        disk_used = disk_total - disk_free;
        disk_percent = ((double)disk_used / disk_total) * 100;
    }
}