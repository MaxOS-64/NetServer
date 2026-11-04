/**
 * @file main.cpp
 * @brief Entry point for the MaxOS NetServer program
 *
 * @date 31st October 2026
 * @author Max Tyson
 */

#include <cstdint>
#include <libkpi/syscalls.h>
#include <interface/ethernet.h>
#include <libnet/protocol/link/arp.h>
#include <../include/network/manager.h>
#include <protocol/link/arp.h>
#include <protocol/network/icmp.h>
#include <protocol/transport/tcp.h>


using namespace MaxOS::KPI;
using namespace NetServer;
using namespace NetServer::protocol;
using namespace NetServer::protocol::link;
using namespace NetServer::protocol::network;
using namespace NetServer::protocol::transport;
using namespace NetServer::interface;
using namespace NetServer::network;
using namespace LibNet;

extern "C" void _start(int argc, char* argv[]){

    klog("Setting up NetServer\n");

    EthernetInterface en0 = EthernetInterface("id");

    NetworkManager network;
    network.add_interface(&en0);

    while (true){};


}