//
// Created by Max Tyson on 05/11/2026.
//

#include <network/route.h>

using namespace NetServer;
using namespace NetServer::network;
using namespace LibNet;
using namespace LibNet::address;

RoutingTable::RoutingTable() = default;

RoutingTable::~RoutingTable() = default;

const route_t* RoutingTable::lookup(ipv4_address_t destination) {

    const route_t* best_route = nullptr;
    uint8_t best_prefix_length = 0;

    for (auto& route : m_routes) {

        if (!route.subnet.contains(destination))
            continue;

        uint8_t prefix_length = route.subnet.prefix_length();

        if (!best_route || prefix_length > best_prefix_length) {
            best_route = &route;
            best_prefix_length = prefix_length;
        }
    }

    return best_route;
}

void RoutingTable::add_route(route_t& route) {
    m_routes.push_back(route);
}

void RoutingTable::remove_route(route_t& route) {
    m_routes.erase(route);
}


