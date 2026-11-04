/**
 * @file udp.cpp
 * @brief Implementation of the User Datagram Protocol (UDP) handler and socket classes
 *
 * @date 24 November 2022
 * @author Max Tyson
 */

#include <protocol/transport/udp.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::KPI;
using namespace LibNet;
using namespace LibNet::protocol;
using namespace LibNet::protocol::transport;
using namespace LibNet::protocol::network;
using namespace LibNet::address;
using namespace NetServer;
using namespace NetServer::interface;
using namespace NetServer::protocol;
using namespace NetServer::protocol::transport;


UDPPacketHandler::UDPPacketHandler() = default;

UDPPacketHandler::~UDPPacketHandler() = default;

/**
 * @brief Handle the recivement of a UDP message
 *
 * @param socket The socket that received the message
 * @param data The data received
 * @param size The size of the data received
 */
void UDPPacketHandler::handle_packet(UDPSocket* socket, uint8_t* data, uint16_t size) {
}


UDPSocket::UDPSocket(UserDatagramProtocol* protocol_provider)
: m_protocol_provider(protocol_provider)
{

}

UDPSocket::~UDPSocket() = default;

/**
 * @brief Handle the recivement of a UDP payload
 *
 * @param data The data received
 * @param size The size of the data received
 */
void UDPSocket::handle_udp_payload(uint8_t* data, uint16_t size) {

    for (const auto& handler : m_packet_handlers)
        handler->handle_packet(this, data, size);
}

/**
 * @brief send data through the UDP socket
 *
 * @param data The data to send
 * @param size The size of the data
 */
void UDPSocket::send(uint8_t* data, uint16_t size) {

    m_protocol_provider->send(this, data, size);

}

/**
 * @brief disconnect the UDP socket
 */
void UDPSocket::close() {

    m_protocol_provider->socket_closed(this);

}

void UserDatagramProtocol::handle_new_connection(UDPSocket* socket, ipv4_address_t remote_ip, port_t remote_port) {

    socket->listening = false;
    socket->remote_ip = remote_ip;
    socket->remote_port = remote_port;

}

/**
 * @brief Construct a new User Datagram Protocol Handler object
 *
 * @param internet_protocol_handler The Internet protocol handler
 * @param error_messages Where to write error messages
 */
UserDatagramProtocol::UserDatagramProtocol(Interface* interface, network::InternetProtocolVersion4* protocol_backend)
: IPV4PacketHandler(protocol_backend, ip_protocol_t::UDP),
  m_interface(interface)
{
}

UserDatagramProtocol::~UserDatagramProtocol() = default;

/**
 * @brief Handle the recivement of an UDP packet
 *
 * @param source_ip The source IP address in big endian
 * @param destination_ip  The destination IP address in big endian
 * @param payload_data The UDP payload
 * @param size The size of the UDP payload
 * @return True if the packet is to be sent back to the sender
 */
bool UserDatagramProtocol::handle_packet(ipv4_address_t source, ipv4_address_t destination, uint8_t* internetprotocol_payload, uint32_t size) {

    recv_udp_packet(source, destination, internetprotocol_payload, size);

    // UDP doesnt return packets
    return false;

}


/**
 * @brief Sends a packet to the remote IP and port
 *
 * @param socket The socket to send the packet from
 * @param data The data to send
 * @param size The size of the data
 */
void UserDatagramProtocol::send_udp_packet(UDPSocket* socket, const uint8_t* data, uint16_t size) {

    // Allocate space for packet
    uint16_t total_size = sizeof(udp_header_t) + size;
    auto* buffer = new uint8_t[total_size];
    auto* header = (udp_header_t*)buffer;

    // Set the header
    header->source_port = host_to_network(socket->local_port.raw_port());
    header->destination_port = host_to_network(socket->remote_port.raw_port());
    header->length = host_to_network(total_size);

    // Fill payload
    if (data != nullptr)
        memcpy(buffer + sizeof(udp_header_t), data, size);

    // Set the checksum (udp doesnt require one)
    header->checksum = 0;

    send(socket->remote_ip, buffer, total_size);
    delete[] buffer;
}


void UserDatagramProtocol::recv_udp_packet(ipv4_address_t source_ip, ipv4_address_t destination_ip, uint8_t* internetprotocol_payload, uint32_t size) {

    // Cant be a UDP packet
    if (size < sizeof(udp_header_t))
        return;

    auto* header = (udp_header_t*)internetprotocol_payload;
    uint16_t local_port = header->destination_port;
    uint16_t remote_port = header->source_port;

    // Find the socket responsible for the connection
    UDPSocket* socket = nullptr;
    for (auto& current_socket : m_connected_sockets) {

        // Not addressed to a port on this machine
        if (current_socket->local_port.raw_port() != network_to_host(local_port) && current_socket->local_ip != destination_ip)
            continue;

        // Socket waiting for connection
        if (socket->listening) {
            socket = current_socket;
            handle_new_connection(socket, source_ip, remote_port);
            continue;
        }

        // This socket is connected to a different end point peer
        if (current_socket->remote_port.raw_port() != network_to_host(remote_port) && current_socket->remote_ip != source_ip)
            continue;

        socket = current_socket;
    }

    // No socket to handle connection
    if (socket == nullptr)
        return;

    socket->handle_udp_payload(internetprotocol_payload + sizeof(udp_header_t), size - sizeof(udp_header_t));
}


/**
 * @brief Connects to a remote host through the UDP protocol
 *
 * @param address The address to connect to in the form "IP:PORT"
 * @return The socket that is connected to the remote host, nullptr if it failed
 */
UDPSocket* UserDatagramProtocol::connect(const string& address) {

    auto parts = address.split(":");
    if (parts.size() != 2)
        return nullptr;

    return connect(parts[0], parts[1].to_uint64());

}

/**
 * Connects the socket to the remote IP and port
 * @param ip The remote IP address in big endian
 * @param port The remote port
 * @return The socket that was connected
 */
UDPSocket* UserDatagramProtocol::connect(ipv4_address_t ip, port_t port) {

    // Create socket to handle this connection
    auto* socket = new UDPSocket(this);
    if (socket == nullptr)
        return nullptr;

    // Configure the socket
    socket->remote_port = port;
    socket->remote_ip = ip;
    socket->local_port = m_free_port++;
    socket->local_ip = m_interface->get_ipv4_address();

    // Wait for server to push datagrams
    m_connected_sockets.push_back(socket);
    return socket;
}

/**
 * @brief Listens for incoming packets on the port
 *
 * @param port The port to listen on
 * @return The socket that is listening
 */
UDPSocket* UserDatagramProtocol::listen(port_t port) {

    // Create socket to handle the connection when it arrives
    auto* socket = new UDPSocket(this);
    if (socket == nullptr)
        return nullptr;

    // Configure the socket
    socket->listening = true;
    socket->local_port = m_free_port++;
    socket->local_ip = m_interface->get_ipv4_address();

    // Wait for client to connect
    m_connected_sockets.push_back(socket);
    return socket;

}