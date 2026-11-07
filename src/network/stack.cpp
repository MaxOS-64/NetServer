//
// Created by Max Tyson on 05/11/2026.
//

#include <network/stack.h>

using namespace LibNet;
using namespace NetServer;
using namespace NetServer::interface;
using namespace NetServer::network;
using namespace NetServer::protocol::link;
using namespace NetServer::protocol::network;
using namespace NetServer::protocol::transport;

NetworkStack::NetworkStack(Interface* interface, RoutingTable* routing_table)
: m_interface(interface)
{

    // Link Layer
    m_ethernet = new EthernetFrameProtocol(m_interface);
    m_arp = new AddressResolutionProtocol(m_interface, m_ethernet);

    // Network Later
    m_ipv4 = new InternetProtocolVersion4(m_interface, m_ethernet, m_arp, routing_table);
    m_icmp = new InternetControlMessageProtocol(m_ipv4);

    // Link layer
    m_tcp = new TransmissionControlProtocol(m_interface, m_ipv4);
    m_udp = new UserDatagramProtocol(m_interface, m_ipv4);
}

NetworkStack::~NetworkStack() = default;

Interface* NetworkStack::interface() const {

    return m_interface;

}

EthernetFrameProtocol* NetworkStack::ethernet() const {
    return m_ethernet;
}

AddressResolutionProtocol* NetworkStack::arp() const {
    return m_arp;
}

InternetProtocolVersion4* NetworkStack::ipv4() const {
    return m_ipv4;
}

InternetControlMessageProtocol* NetworkStack::icmp() const {
    return m_icmp;
}

TransmissionControlProtocol* NetworkStack::tcp() const {
    return m_tcp;
}

UserDatagramProtocol* NetworkStack::udp() const {
    return m_udp;
}
