//
// Created by Max Tyson on 02/11/2026.
//

#ifndef NETSERVER_ROUTE_H
#define NETSERVER_ROUTE_H

#include <libnet/address/ipv4.h>
#include <interface/interface.h>

namespace NetServer::network {

    typedef struct Route {

        LibNet::address::Subnet destination;
        LibNet::address::ipv4_address_t gateway;
        interface::Interface* interface;

    } route_t;

    class RoutingTable {

        private:
            MaxOS::common::Vector<Route> m_routes;

        public:
            const Route* lookup(LibNet::address::ipv4_address_t destination);

            void add_route(const Route& route);
            void remove_route(const Route& route);
    };

}

#endif //NETSERVER_ROUTE_H
