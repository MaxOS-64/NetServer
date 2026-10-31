/**
 * @file main.cpp
 * @brief Entry point for the MaxOS NetServer program
 *
 * @date 31st October 2026
 * @author Max Tyson
 */

#include <cstdint>
#include <libkpi/syscalls.h>

using namespace MaxOS::KPI;

extern "C" void _start(int argc, char* argv[]){

    klog("Setting up NetServer\n");
    while (true){};

}