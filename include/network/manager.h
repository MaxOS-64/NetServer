//
// Created by Max Tyson on 02/11/2026.
//

#ifndef NETSERVER_NETWORK_H
#define NETSERVER_NETWORK_H

#include <network/stack.h>
#include <network/route.h>

namespace NetServer::network {

    class NetworkManager {

        private:
            RoutingTable m_routing_table;

            MaxOS::common::Map<interface::Interface*, NetworkStack*> m_interfaces;

        public:
            NetworkManager();
            ~NetworkManager();

            void add_interface(interface::Interface* interface);
            void remove_interface(interface::Interface* interface);

            RoutingTable& get_routing_table();

    };

}

#endif //NETSERVER_STACK_H
