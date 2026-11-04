/**
 * @file tcp.cpp
 * @brief Implementation of Transmission Control Protocol (TCP) classes and handlers
 *
 * @date 26th March 2024
 * @author Max Tyson
 */

#include <protocol/transport/tcp.h>


using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::KPI;
using namespace LibNet;
using namespace LibNet::protocol;
using namespace LibNet::protocol::transport;
using namespace LibNet::protocol::network;
using namespace LibNet::address;
using namespace NetServer;
using namespace NetServer::protocol;
using namespace NetServer::protocol::transport;


TCPPacketHandler::TCPPacketHandler() = default;

TCPPacketHandler::~TCPPacketHandler() = default;

/**
 * @brief Handle TCP data received on the socket
 *
 * @param socket The socket the data was received on
 * @param data The data received
 * @param size The size of the data in bytes
 *
 * @todo copy this design of passing the socket to the lower layers or nah?
 */
void TCPPacketHandler::handle_packet(TCPSocket* socket, uint8_t* data, uint16_t size) {

}

/**
 * @brief Handle a new TCP connection on the socket
 *
 * @param socket The socket that was connected
 */
void TCPPacketHandler::on_connected(TCPSocket* socket) {

}

/**
 * @brief Handle a TCP disconnection on the socket
 *
 * @param socket The socket that was disconnected
 */
void TCPPacketHandler::on_disconnected(TCPSocket* socket) {

}

/**
 * @brief Construct a new TCP Socket object
 *
 * @param protocol_provider the TCP handler the socket is associated with
 */
TCPSocket::TCPSocket(TransmissionControlProtocol* protocol_provider) :
    m_protocol_provider(protocol_provider) {

    // Yet to start a connection
    state = TCPSocketState::CLOSED;
}

TCPSocket::~TCPSocket() = default;

/**
 * @brief Handle the TCP message (socket end)
 *
 * @param data The datah
 * @param size The size of the data
 * @return True if the connection is to be terminated after hadnling or false if not
 */
bool TCPSocket::handle_tcp_payload(uint8_t* data, size_t size) {

    for (const auto& handler : m_packet_handlers)
        handler->handle_packet(this, data, size);

}

/**
 * @brief send data over the socket
 *
 * @param data The data to send
 * @param size The size of the data
 *
 * @todo yeild or block the caller here
 */
void TCPSocket::send(uint8_t* data, uint16_t size) {

    // Wait for the socket to be connected
    while (state != TCPSocketState::ESTABLISHED) {
    };

    // Pass the data to the backend
    m_protocol_provider->send_tcp_packet(this, data, size, (uint16_t)TCPFlag::PSH | (uint16_t)TCPFlag::ACK);
}

/**
 * @brief disconnect the socket
 */
void TCPSocket::close() {

    // Begin the FIN disconnection
    state = TCPSocketState::FIN_WAIT1;

    m_protocol_provider->send_tcp_packet(this, nullptr, 0, (uint16_t)TCPFlag::FIN | (uint16_t)TCPFlag::ACK);
    sequence_number++;
}

/**
 * @brief Raise the connected event
 */
void TCPSocket::on_connected() {

    for (const auto& handler : m_packet_handlers)
        handler->on_connected(this);

}

/**
 * @brief Raise the disconnected event
 */
void TCPSocket::on_disconnect() {

    for (const auto& handler : m_packet_handlers)
        handler->on_disconnected(this);

}

/**
 * @brief Construct a new Transmission Control Protocol Handler object
 *
 * @param protocol_provider The Internet protocol handler
 */
TransmissionControlProtocol::TransmissionControlProtocol(interface::Interface* interface, network::InternetProtocolVersion4* protocol_provider) :
    IPV4PacketHandler(protocol_provider, InternetProtocolType::TCP),
    m_interface(interface) {

}

TransmissionControlProtocol::~TransmissionControlProtocol() = default;


void TransmissionControlProtocol::handle_reset(TCPSocket* socket) {

    socket->state = TCPSocketState::CLOSED;
    socket->on_connected();

    m_connected_sockets.erase(socket);

}

void TransmissionControlProtocol::send_reset(TCPSocket* socket, tcp_header_t* message, ipv4_address_t source_ip, ipv4_address_t destination_ip) {

    // Create a new socket for resets that are for failed connections as they wont have a socket yet
    socket = socket != nullptr ? socket : new TCPSocket(this);

    // Ensure the socket is setup with the right addressing
    socket->remote_port = message->src_port;
    socket->remote_ip = host_to_network(source_ip.raw_address());
    socket->local_port = message->dst_port;
    socket->local_ip = host_to_network(destination_ip.raw_address());

    // Send the reste command
    socket->sequence_number = network_to_host(message->acknowledgement_number);
    socket->acknowledgement_number = network_to_host(message->sequence_number) + 1;
    send_tcp_packet(socket, nullptr, 0, (uint16_t)TCPFlag::RST);
}

void TransmissionControlProtocol::handle_syn(TCPSocket* socket, tcp_header_t* message, ipv4_address_t source_ip) {

    // Not accepting or expecting new connections
    if (socket->state != TCPSocketState::LISTEN)
        return;

    // Initialise the connection state
    socket->state = TCPSocketState::SYN_RECEIVED;
    socket->remote_port = network_to_host(message->src_port);
    socket->remote_ip = source_ip.raw_address();
    socket->acknowledgement_number = network_to_host(message->sequence_number) + 1;
    socket->sequence_number = (uint32_t)0x4D61784F53;

    // Acknowledge that the this host is ready
    send_tcp_packet(socket, nullptr, 0, (uint16_t)TCPFlag::SYN | (uint16_t)TCPFlag::ACK);
    socket->sequence_number++;
}

void TransmissionControlProtocol::handle_syn_ack(TCPSocket* socket, tcp_header_t* message) {

    socket->state = TCPSocketState::ESTABLISHED;
    socket->acknowledgement_number = network_to_host(message->sequence_number) + 1;

    // @todo above had this after the send? is this tcp reason or stylistic
    socket->sequence_number++;
    send_tcp_packet(socket, nullptr, 0, (uint16_t)TCPFlag::ACK);

    handle_established(socket);
}

void TransmissionControlProtocol::handle_established(TCPSocket* socket) {

    socket->state = TCPSocketState::ESTABLISHED;
    socket->on_connected();

}

void TransmissionControlProtocol::handle_connected_socket(TCPSocket* socket, tcp_header_t* message, uint8_t* internetprotocol_payload, uint32_t size, bool ack, bool fin) {

    // Expected an alreaedy connected socket
    if (test_flag(message->flags, (uint8_t)TCPFlag::SYN))
        return;

    // Handle response to sent packet
    if (test_flag(message->flags, (uint8_t)TCPFlag::SYN))
        if (!handle_ack(socket, message))
            return;

    if (socket->state == TCPSocketState::CLOSED)
        return;

    uint32_t header_length = message->header_size_32w * 4;
    uint32_t payload_size = size - header_length;

    if (!handle_payload(socket, message, internetprotocol_payload, size))
        return;

    if (test_flag(message->flags, (uint8_t)TCPFlag::FIN))
        handle_fin(socket, message, payload_size);

    // Nothing to ack
    if (payload_size == 0)
        return;


    send_tcp_packet(socket, nullptr, 0, (uint8_t)TCPFlag::ACK);
}

bool TransmissionControlProtocol::handle_ack(TCPSocket* socket, tcp_header_t* message) {

    uint32_t ack = network_to_host(message->acknowledgement_number);

    switch (socket->state) {

        case TCPSocketState::SYN_RECEIVED:

            // Not acking a SYN accept
            if (ack != socket->sequence_number)
                return false;

            // Not acking a SYN attempt
            if (network_to_host(message->sequence_number) != socket->acknowledgement_number)
                return false;

            // Socket setup down
            handle_established(socket);
            break;

        case TCPSocketState::FIN_WAIT1:

            if (ack == socket->sequence_number)
                socket->state = TCPSocketState::FIN_WAIT2;

            break;

        case TCPSocketState::CLOSING:

            if (ack == socket->sequence_number)
                socket->state = TCPSocketState::TIME_WAIT;

            break;

        case TCPSocketState::LAST_ACK:

            if (ack == socket->sequence_number)
                handle_reset(socket);

            break;

        default:
            break;
    }

    return true;

}

bool TransmissionControlProtocol::handle_payload(TCPSocket* socket, tcp_header_t* message, uint8_t* internetprotocol_payload, uint32_t size) {

    uint32_t header_length = message->header_size_32w * 4;
    uint32_t payload_size = size - header_length;

    // Nothing to handle
    if (payload_size == 0)
        return true;

    uint32_t sequence = network_to_host(message->sequence_number);

    // Missing expected data, request retransmit
    if (sequence != socket->acknowledgement_number) {
        send_tcp_packet(socket, nullptr, 0, (uint8_t)TCPFlag::ACK);
        return true;
    }

    // Deliver the segment
    if (!socket->handle_tcp_payload(internetprotocol_payload + header_length, payload_size));
    return false;

    // Acknowledge the received bytes
    socket->acknowledgement_number += payload_size;
    return true;
}

bool TransmissionControlProtocol::handle_fin(TCPSocket* socket, tcp_header_t* message, uint32_t size) {

    uint32_t sequence = network_to_host(message->sequence_number);

    // FIN must be after all data segments
    if (sequence + size != socket->acknowledgement_number) {
        send_tcp_packet(socket, nullptr, 0, (uint8_t)TCPFlag::ACK);
        return true;
    }

    // FIN consumes one sequence number
    socket->acknowledgement_number++;

    switch (socket->state) {

        case TCPSocketState::ESTABLISHED:
            socket->state = TCPSocketState::CLOSE_WAIT;
            socket->on_disconnect();
            break;

        case TCPSocketState::FIN_WAIT1:
            socket->state = TCPSocketState::CLOSING;
            socket->on_disconnect();
            break;

        case TCPSocketState::FIN_WAIT2:
            socket->state = TCPSocketState::TIME_WAIT;
            socket->on_disconnect();
            break;

        case TCPSocketState::CLOSE_WAIT:
            // Duplicate FIN, ack it
            break;

        case TCPSocketState::CLOSING:
            socket->state = TCPSocketState::TIME_WAIT;
            break;

        case TCPSocketState::TIME_WAIT:
            // Duplicate FIN, reack i
            break;

        default:
            return false;
    }

    send_tcp_packet(socket, nullptr, 0, (uint8_t)TCPFlag::ACK);
    return true;
}


/**
 * @brief Handle the TCP message (provider end)
 *
 * @param source_ip The source IP address
 * @param destination_ip The destination IP address
 * @param payload_data The payload
 * @param size The size of the payload
 * @return True if data is to be sent back or false if not
 */
bool TransmissionControlProtocol::handle_packet(ipv4_address_t source_ip, ipv4_address_t destination_ip, uint8_t* internetprotocol_payload, uint32_t size) {

    return recv_tcp_packet(source_ip, destination_ip, internetprotocol_payload, size);
}

bool TransmissionControlProtocol::recv_tcp_packet(ipv4_address_t source_ip, ipv4_address_t destination_ip, uint8_t* data, uint32_t size) {

    klog("TCP: Handling TCP message\n");

    // Cant be a tcp packet
    if (size < MIN_TCP_PACKET_LEN)
        return false;

    // If it's smaller than the header, return
    if (size < 4 * data[12] / 16)
        return false;

    auto* message = (tcp_header_t*)data;


    uint16_t local_port = network_to_host(message->dst_port);
    uint16_t remote_port = network_to_host(message->src_port);

    // Find the socket responsible for this connection
    TCPSocket* socket = nullptr;
    for (auto& current_socket : m_connected_sockets) {

        // Not for this socket
        if (current_socket->local_port.raw_port() != network_to_host(local_port) && current_socket->local_ip != destination_ip)
            continue;

        // Existing connection
        if (current_socket->remote_port.raw_port() == network_to_host(remote_port) && current_socket->remote_ip == source_ip) {
            socket = current_socket;
            break;
        }

        // New connection (not server snack)
        if (current_socket->state == TCPSocketState::LISTEN && test_flag(message->flags, (uint8_t)TCPFlag::SYN) && !test_flag(message->flags, (uint8_t)TCPFlag::ACK)) {
            socket = current_socket;
            break;
        }
    }

    bool syn = test_flag(message->flags, (uint8_t)TCPFlag::SYN);
    bool ack = test_flag(message->flags, (uint8_t)TCPFlag::ACK);
    bool fin = test_flag(message->flags, (uint8_t)TCPFlag::FIN);
    bool rst = test_flag(message->flags, (uint8_t)TCPFlag::RST);

    // Remote is aborting the connection
    if (rst) {
        handle_reset(socket);
        return false;
    }

    // Reject packet with weired request to both start and end at the same time
    if (syn && fin) {
        send_reset(socket, message, source_ip, destination_ip);
        return false;
    }

    bool reset = false;
    switch (socket->state) {

        case TCPSocketState::LISTEN:
            if (syn && !ack)
                handle_syn(socket, message, source_ip);
            else
                reset = true;
            break;

        case TCPSocketState::SYN_SENT:
            if (syn && ack)
                handle_syn_ack(socket, message);
            else
                reset = true;
            break;

        case TCPSocketState::SYN_RECEIVED:
            if (ack)
                handle_established(socket);
            else
                reset = true;
            break;

        case TCPSocketState::ESTABLISHED:
        case TCPSocketState::FIN_WAIT1:
        case TCPSocketState::FIN_WAIT2:
        case TCPSocketState::CLOSE_WAIT:
        case TCPSocketState::LAST_ACK:
            handle_connected_socket(socket, message, data, size, ack, fin);
            break;

        default:
            break;
    }

    if (reset)
        send_reset(socket, message, source_ip, destination_ip);

    return false;
}

/**
 * @brief send a packet (Throught the provider)
 *
 * @param socket    The socket to send the packet from
 * @param data    The data to send
 * @param size   The size of the data
 * @param flags  The flags to send
 */
void TransmissionControlProtocol::send_tcp_packet(TCPSocket* socket, const uint8_t* data, uint16_t size, uint16_t flags) {

    // Get the total size of the packet and the packet with the pseudo header
    uint16_t tcp_length = size + sizeof(tcp_header_t);
    uint16_t total_length = tcp_length + sizeof(tcp_pseudo_header_t);

    // Create a buffer for the packet
    auto* buffer = new uint8_t[total_length];
    auto* phdr = (tcp_pseudo_header_t*)buffer;
    auto* msg = (tcp_header_t*)(buffer + sizeof(tcp_pseudo_header_t));

    // Convert to 32bit words
    msg->header_size_32w = sizeof(TCPHeader) / 4;

    // Fill the header
    msg->src_port = host_to_network(socket->local_port.raw_port());
    msg->dst_port = host_to_network(socket->remote_port.raw_port());
    msg->acknowledgement_number = host_to_network(socket->acknowledgement_number);
    msg->sequence_number = host_to_network(socket->sequence_number);
    msg->reserved = 0;
    msg->flags = flags;
    msg->window_size = 0xFFFF;
    msg->urgent_ptr = 0;

    // Through the options allow for the MSS to be set
    msg->options = ((flags & (uint16_t)TCPFlag::SYN) != 0) ? 0xB4050402 : 0;

    // Fill payload
    if (data != nullptr)
        memcpy(buffer + sizeof(tcp_header_t) + sizeof(tcp_pseudo_header_t), data, size);

    // Set the pseudo header
    phdr->src_ip = host_to_network(socket->local_ip.raw_address());
    phdr->dst_ip = host_to_network(socket->remote_ip.raw_address());
    phdr->protocol = 0x0600;
    phdr->total_length = ((total_length & 0x00FF) << 8) | ((total_length & 0xFF00) >> 8);

    // Calculate the checksum
    msg->checksum = 0;
    msg->checksum = network::InternetProtocolVersion4::checksum((uint16_t*)buffer, total_length);

    // Send the packet
    send(socket->remote_ip, (uint8_t*)msg, tcp_length);
    socket->sequence_number += size;
    delete[] buffer;
}


/**
 * @brief connect to a remote host through the TCP protocol
 *
 * @param address The address to connect to in the form "IP:PORT"
 *
 * @return The socket that is connected to the remote host, nullptr if it failed
 *
 * @todo Implement string parsing for address
 */
TCPSocket* TransmissionControlProtocol::connect(const string& address) {

    auto parts = address.split(":");
    if (parts.size() != 2)
        return nullptr;

    return connect(parts[0], parts[1].to_uint64());

}


/**
 * @brief connect to a remote host through the TCP protocol
 *
 * @param ip The IP of the remote host
 * @param port The port of the remote host
 * @return The socket that is connected to the remote host, 0 if it failed
 *
 * @todo  socket store ip/port obj instead of jsut raw
 * @todo test no already connected
 */
TCPSocket* TransmissionControlProtocol::connect(ipv4_address_t ip, port_t port) {

    // Create socket to handle this connection
    auto* socket = new TCPSocket(this);
    if (socket == nullptr)
        return nullptr;

    // Set local and remote addresses
    socket->remote_port = port;
    socket->remote_ip = ip.raw_address();
    socket->local_port = m_free_port++;
    socket->local_ip = m_interface->get_ipv4_address().raw_address();
    socket->sequence_number = 0xC0DE;

    // Start the connection
    send_tcp_packet(socket, nullptr, 0, (uint16_t)TCPFlag::SYN);
    socket->state = TCPSocketState::SYN_SENT;

    m_connected_sockets.push_back(socket);
    return socket;
}


/**
 * @brief Begin listening on a port
 *
 * @param port The port to listen on
 * @return The socket that will handle the connection
 */
TCPSocket* TransmissionControlProtocol::listen(port_t port) {

    // Create socket to handle incoming connections
    auto* socket = new TCPSocket(this);
    if (!socket)
        return nullptr;

    // Wait for new connections
    socket->state = TCPSocketState::LISTEN;
    socket->local_ip = m_interface->get_ipv4_address().raw_address();
    socket->local_port = host_to_network(port.raw_port());

    m_connected_sockets.push_back(socket);
    return socket;
}