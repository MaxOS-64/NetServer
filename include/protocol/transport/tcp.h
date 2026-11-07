/**
 * @file tcp.h
 * @brief Defines the Transmission Control Protocol (TCP) structures and classes for handling TCP sockets and payloads.
 *
 * @date 12th September 2022
 * @author Max Tyson
 */

#ifndef NETSERVER_PROTOCOL_TRANSPORT_TCP_H
#define NETSERVER_PROTOCOL_TRANSPORT_TCP_H

#include <cstdint>
#include <protocol/network/ipv4.h>
#include <libnet/protocol/transport/tcp.h>
#include <libcommon/mem.h>
#include <libcommon/bits.h>
#include <libnet/address/port.h>

namespace NetServer::protocol::transport {


	// Forward declarations
	class TransmissionControlProtocol;
	class TCPSocket;

    /**
     * @class TCPPayloadHandler
     * @brief Handler for TCP payloads
     */
    class TCPPacketHandler {
    public:
        TCPPacketHandler();
        ~TCPPacketHandler();

        virtual void handle_packet(TCPSocket* socket, uint8_t* data, uint16_t size);
        virtual void on_connected(TCPSocket* socket);
        virtual void on_disconnected(TCPSocket* socket);
    };

    /**
     * @class TCPSocket
     * @brief A TCP socket. Allows for sending and receiving data over TCP at a port
     */
    class TCPSocket {
        friend class TransmissionControlProtocol;

        private:

            TransmissionControlProtocol* m_protocol_provider;  ///< The TCP handler this socket is using
            LibNet::protocol::transport::TCPSocketState state;                                                    ///< The state of the socket

            MaxOS::common::Vector<TCPPacketHandler*> m_packet_handlers;


        protected:
            LibNet::address::ipv4_address_t local_ip  = 0;                       ///< The IP address of this device
            LibNet::address::ipv4_address_t remote_ip = 0;                      ///< The IP address of the external device
            LibNet::address::port_t local_port  = 0;                     ///< The port on this device
            LibNet::address::port_t remote_port = 0;                    ///< The port on the external device

            uint32_t sequence_number = 0;                ///< The current order number of the data being sent
            uint32_t acknowledgement_number = 0;         ///< The number used to keep track of what has been received, incremented by 1 each time

        public:
            explicit TCPSocket(TransmissionControlProtocol* protocol_provider);
            ~TCPSocket();

            void send(uint8_t* data, uint16_t size);
            void close();

            void on_connected();
            void on_disconnect();

            void add_handler(TCPPacketHandler* handler);
            void remove_handler(TCPPacketHandler* handler);

            bool handle_tcp_payload(uint8_t* data, size_t size);
    };



    /**
	 * @class TransmissionControlProtocol
	 * @brief Handles TCP packets and manages TCP sockets
	 */
    class TransmissionControlProtocol : network::IPV4PacketHandler {
        friend class TCPSocket;

        private:
            MaxOS::common::Vector<TCPSocket*> m_connected_sockets;         ///< The list of connected sockets
            inline static uint16_t m_free_port = 0x8000;                    ///< The next free port to use for new sockets (@todo this needs to be somewhere else)
            interface::Interface* m_interface = nullptr;

            void handle_reset(TCPSocket* socket);
            void send_reset(TCPSocket* socket, LibNet::protocol::transport::tcp_header_t* message, LibNet::address::ipv4_address_t source_ip, LibNet::address::ipv4_address_t destination_ip);
            void handle_syn(TCPSocket* socket, LibNet::protocol::transport::tcp_header_t* message, LibNet::address::ipv4_address_t source_ip);
            void handle_syn_ack(TCPSocket* socket, LibNet::protocol::transport::tcp_header_t* message);
            void handle_established(TCPSocket* socket);
            void handle_connected_socket(TCPSocket* socket, LibNet::protocol::transport::tcp_header_t* message, uint8_t* internetprotocol_payload, uint32_t size, bool ack, bool fin);
            bool handle_ack(TCPSocket* socket, LibNet::protocol::transport::tcp_header_t* message);
            bool handle_payload(TCPSocket* socket, LibNet::protocol::transport::tcp_header_t* message, uint8_t* internetprotocol_payload, uint32_t size);
            bool handle_fin(TCPSocket* socket, LibNet::protocol::transport::tcp_header_t* message, uint32_t size);

        public:
            TransmissionControlProtocol(interface::Interface* interface, network::InternetProtocolVersion4* protocol_provider);
            ~TransmissionControlProtocol();

            bool handle_packet(LibNet::address::ipv4_address_t source_ip, LibNet::address::ipv4_address_t destination_ip, uint8_t* internetprotocol_payload, uint32_t size) override;

            void send_tcp_packet(TCPSocket* socket, const uint8_t* data, uint16_t size, uint16_t flags = 0);
            bool recv_tcp_packet(LibNet::address::ipv4_address_t source, LibNet::address::ipv4_address_t destination, uint8_t* internetprotocol_payload, uint32_t size);

            TCPSocket* connect(const MaxOS::string& address);
            TCPSocket* connect(LibNet::address::ipv4_address_t ip, LibNet::address::port_t port);
            TCPSocket* listen(LibNet::address::port_t port);
    };
}


#endif //NETSERVER_PROTOCOL_TRANSPORT_TCP_H
