/**
 * @file ipv4.cpp
 * @brief Implementation of IPv4 protocol handlers
 *
 * @date 22nd November 2022
 * @author Max Tyson
 */

#include <protocol/network/ipv4.h>


using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::KPI;
using namespace LibNet;
using namespace LibNet::protocol;
using namespace LibNet::protocol::link;
using namespace LibNet::protocol::network;
using namespace LibNet::address;
using namespace NetServer;
using namespace NetServer::network;
using namespace NetServer::protocol;
using namespace NetServer::protocol::network;

/**
 * @brief Construct a new IPV4 Payload Handler object and register it with the Internet Protocol Handler
 *
 * @param protocol_provider The Internet Protocol Handler to register with
 * @param handled_protocol The IP protocol to handle
 */
IPV4PacketHandler::IPV4PacketHandler(InternetProtocolVersion4* protocol_provider, ip_protocol_t handled_protocol) :
    m_protocol_provider(protocol_provider),
    m_handled_protocol(handled_protocol) {
    protocol_provider->add_handler(this);
}

IPV4PacketHandler::~IPV4PacketHandler() = default;

/**
 * @brief Sends an IP packet.
 *
 * @param destination_ip The destination IP address.
 * @param payload_data The payload of the IP packet.
 * @param size The size of the IP packet.
 */
void IPV4PacketHandler::send(ipv4_address_t destination_ip, uint8_t* payload_data, uint32_t size) {

    // Pass to backend
    m_protocol_provider->send_ip_packet(destination_ip, m_handled_protocol, payload_data, size);

}

/**
 * @brief Called when an IP packet is received. (Deafult, does nothing, overide for use)
 *
 * @param source The source IP address.
 * @param destination The destination IP address.
 * @param internetprotocol_payload The payload of the IP packet.
 * @param size The size of the IP packet.
 * @return True if the packet was handled, false otherwise.
 */
bool IPV4PacketHandler::handle_packet(ipv4_address_t source, ipv4_address_t destination, uint8_t* internetprotocol_payload, uint32_t size) {

    return false;
}


/**
 * @brief Construct a new Internet Protocol Handler object
 *
 * @param backend The backend Ethernet frame handler.
 * @param own_internet_protocol_address The IP address of this device.
 * @param default_gateway_internet_protocol_address The IP address of the default gateway.
 * @param subnet_mask The subnet mask.
 * @param error_messages Where to write error messages.
 */
InternetProtocolVersion4::InternetProtocolVersion4(interface::Interface* interface, link::EthernetFrameProtocol* protocol_provider, IPV4AddressResolver* resolver, RoutingTable* router) :
    EthernetFramePacketHandler(protocol_provider, EthernetFrameType::IPv4),
    m_interface(interface),
    m_resolver(resolver),
    m_router(router) {
}

InternetProtocolVersion4::~InternetProtocolVersion4() = default;

/**
 * @brief Called when an IP packet is received.
 *
 * @param ethernetframe_payload The payload of the IP packet.
 * @param size The size of the IP packet.
 * @return True if the packet is to be sent back, false otherwise.
 *
 * @todo Set the identifier when sending packets back
 */
bool InternetProtocolVersion4::handle_packet(uint8_t* ethernetframe_payload, uint32_t size) {
    return recv_ip_packet(ethernetframe_payload, size);
}


/**
 * @brief Sends an IP packet.
 *
 * @param destination The destination IP address.
 * @param protocol The protocol of the IP packet.
 * @param data The payload of the IP packet.
 * @param size The size of the IP packet.
 *
 * @todo doesnt work for packets bigger than 1500 bytes as doesnt fragment
 * @todo set identification properly
 */
void InternetProtocolVersion4::send_ip_packet(ipv4_address_t destination, ip_protocol_t protocol, const uint8_t* data, uint32_t size) {

    // Create space for the message
    auto* buffer = new uint8_t[sizeof(ipv4_header_t) + size];
    auto* message = (ipv4_header_t*)buffer;

    // Build the message
    message->version = 4;
    message->header_length = sizeof(ipv4_header_t) / 4;
    message->type_of_service = 0;
    message->total_length = host_to_network(size + sizeof(ipv4_header_t));
    message->identifier = 0x100;
    message->flags_and_offset = 0x0040;
    message->time_to_live = 0x40;
    message->protocol = host_to_network((uint8_t)protocol);
    message->destination_ip = host_to_network(destination.raw_address());
    message->source_ip = host_to_network(m_interface->get_ipv4_address().raw_address());

    // CRC the message
    message->checksum = 0;
    message->checksum = checksum((uint16_t*)message, sizeof(IPV4Header));

    // Fill buffer with payload
    memcpy(buffer + sizeof(ipv4_header_t), data, size);

    // Find the address of the next destination for this packet
    const Route* route = m_router->lookup(destination);
    ipv4_address_t next_hop = route->gateway.is_unspecified() ? destination : route->gateway;
    mac_t mac = m_resolver->resolve(next_hop);

    // Send message
    m_protocol_provider->send_ethernet_frame(mac, m_handled_type, buffer, size + sizeof(ipv4_header_t));
    delete[] buffer;
}

/**
 *
 * @param data
 * @param size
 * @return
 *
 * @todo errors with droppping packets
 */
bool InternetProtocolVersion4::recv_ip_packet(uint8_t* data, uint32_t size) {
    klog("IP: Handling packet\n");

    // Cant be a ipv4 packet
    if (size < sizeof(ipv4_header_t))
        return false;

    auto* ip_message = (ipv4_header_t*)data;

    // Not for this device
    if (ip_message->destination_ip != host_to_network(m_interface->get_ipv4_address().raw_address()))
        return false;

    // Prevent heart bleed attacks by not reading past the length of the message (ie dont trust user length))
    uint32_t length = ip_message->total_length;
    if (length > size)
        length = size;

    // Nothing to handle this
    auto handler_iterator = m_packet_handlers.find((ip_protocol_t)network_to_host(ip_message->protocol));
    if (handler_iterator == m_packet_handlers.end() || handler_iterator->second == nullptr)
        return false;

    // Pass packet to upper layer
    bool send_back = handler_iterator->second->handle_packet(network_to_host(ip_message->source_ip), ip_message->destination_ip, data + sizeof(ipv4_header_t), length - sizeof(ipv4_header_t));
    klog("IP: Handled packet\n");

    if (!send_back)
        return false;

    // Reverse addressing to send back to the sender
    uint32_t source = ip_message->destination_ip;
    ip_message->destination_ip = ip_message->source_ip;
    ip_message->source_ip = source;

    // Reset the message state
    ip_message->time_to_live = 0x40;
    ip_message->checksum = checksum((uint16_t*)ip_message, 4 * ip_message->header_length);

    return send_back;
}

/**
 * @brief Creates a checksum for the given data.
 *
 * @param data The data to create a checksum for.
 * @param length_in_bytes The length of the data in bytes.
 * @return The checksum.
 */
uint16_t InternetProtocolVersion4::checksum(const uint16_t* data, uint32_t length_in_bytes) {

    uint32_t sum = 0;

    // Add each 16-bit word in network byte order
    for (uint32_t i = 0; i + 1 < length_in_bytes; i += 2)
        sum += ((data[i]) << 8) | data[i + 1];

    // Handle an odd trailing byte
    if (length_in_bytes % 2)
        sum += (data[length_in_bytes - 1]) << 8;

    // Fold overflow back into the lower 16 bits
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    // Invert and convert to network byte order
    return host_to_network(~sum);
}