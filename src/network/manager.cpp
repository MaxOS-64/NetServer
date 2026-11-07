//
// Created by Max Tyson on 05/11/2026.
//

#include <network/manager.h>

using namespace NetServer;
using namespace NetServer::network;

NetworkManager::NetworkManager() = default;

NetworkManager::~NetworkManager() = default;

void NetworkManager::add_interface(interface::Interface* interface) {
    m_interfaces.push_back(interface, new NetworkStack(interface, &m_routing_table));
}

void NetworkManager::remove_interface(interface::Interface* interface) {
    m_interfaces.erase(interface);
}

RoutingTable& NetworkManager::get_routing_table() {
    return m_routing_table;
}
