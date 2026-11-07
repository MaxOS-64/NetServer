//
// Created by Max Tyson on 02/11/2026.
//

#ifndef MAXOS_STACK_H
#define MAXOS_STACK_H

#include <interface/interface.h>
#include <protocol/link/ethernetframe.h>
#include <protocol/link/arp.h>
#include <protocol/network/ipv4.h>
#include <protocol/network/icmp.h>
#include <protocol/transport/tcp.h>
#include <protocol/transport/udp.h>

namespace NetServer::network {

    //@todo if grows then lazy load

    class NetworkStack {
        private:
            interface::Interface* m_interface;

            protocol::link::EthernetFrameProtocol* m_ethernet;
            protocol::link::AddressResolutionProtocol* m_arp;
            protocol::network::InternetProtocolVersion4* m_ipv4;
            protocol::network::InternetControlMessageProtocol* m_icmp;
            protocol::transport::TransmissionControlProtocol* m_tcp;
            protocol::transport::UserDatagramProtocol* m_udp;

        public:
            explicit NetworkStack(interface::Interface* interface, RoutingTable* routing_table);
            ~NetworkStack();

            [[nodiscard]] interface::Interface* interface() const;

            [[nodiscard]] protocol::link::EthernetFrameProtocol* ethernet() const;
            [[nodiscard]] protocol::link::AddressResolutionProtocol* arp() const;
            [[nodiscard]] protocol::network::InternetProtocolVersion4* ipv4() const;
            [[nodiscard]] protocol::network::InternetControlMessageProtocol* icmp() const;
            [[nodiscard]] protocol::transport::TransmissionControlProtocol* tcp() const;
            [[nodiscard]] protocol::transport::UserDatagramProtocol* udp() const;
    };

};

#endif //MAXOS_STACK_H
