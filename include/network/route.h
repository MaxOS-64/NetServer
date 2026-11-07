//
// Created by Max Tyson on 02/11/2026.
//

#ifndef NETSERVER_ROUTE_H
#define NETSERVER_ROUTE_H

#include <libnet/address/ipv4.h>
#include <interface/interface.h>

namespace NetServer::network {

    typedef struct Route {

        LibNet::address::Subnet subnet;
        LibNet::address::ipv4_address_t gateway;
        interface::Interface* interface;

    } route_t;

    class RoutingTable {

        private:
            MaxOS::common::Vector<route_t> m_routes = {};

        public:

            RoutingTable();
            ~RoutingTable();

            const route_t* lookup(LibNet::address::ipv4_address_t destination);

            void add_route(route_t& route);
            void remove_route(route_t& route);
    };

}

#endif //NETSERVER_ROUTE_H
