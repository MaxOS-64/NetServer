//
// Created by Max Tyson on 02/11/2026.
//

#ifndef MAXOS_STACK_H
#define MAXOS_STACK_H

#include <interface/interface.h>
#include <protocol/link/ethernetframe.h>
#include <protocol/link/arp.h>

namespace NetServer::network {


    class NetworkStack {
        private:
            interface::Interface* m_interface;

            protocol::link::EthernetFrameProtocol* m_ethernet;
            protocol::link::AddressResolutionProtocol* m_arp;

        public:
            explicit NetworkStack(interface::Interface* interface);
            ~NetworkStack();

            bool initialise();

            interface::Interface* interface() const;

            protocol::link::EthernetFrameProtocol* ethernet() const;
            protocol::link::AddressResolutionProtocol* arp() const;

    };

};

#endif //MAXOS_STACK_H
