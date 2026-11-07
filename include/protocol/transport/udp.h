/**
 * @file udp.h
 * @brief Defines the User Datagram Protocol (UDP) for network communication.
 *
 * @date 24th November 2022
 * @author Max Tyson
 */

#ifndef NETSERVER_PROTOCOL_TRANSPORT_UDP_H
#define NETSERVER_PROTOCOL_TRANSPORT_UDP_H

#include <cstdint>
#include <protocol/network/ipv4.h>
#include <libcommon/mem.h>
#include <libnet/protocol/transport/udp.h>
#include <libnet/address/port.h>

namespace NetServer::protocol::transport {

    // Forward declarations
    class UserDatagramProtocol;
    class UDPSocket;

    /**
     * @class UDPPacketHandler
     * @brief Handles the payload of a UDP packet
     */
    class UDPPacketHandler {
        public:
            UDPPacketHandler();
            ~UDPPacketHandler();

            virtual void handle_packet(UDPSocket* socket, uint8_t* data, uint16_t size);
    };

    /**
     * @class UDPSocket
     * @brief A UDP socket
     */
    class UDPSocket {
        friend class UserDatagramProtocol;

        private:
            UserDatagramProtocol* m_protocol_provider; ///< The UDP handler this socket is connected to
            MaxOS::common::Vector<UDPPacketHandler*> m_packet_handlers;

        protected:
            bool listening = false; ///< Wether the port is waiting for incoming connections

            LibNet::address::ipv4_address_t local_ip = 0; ///< The IP of this device
            LibNet::address::ipv4_address_t remote_ip = 0; ///< The IP of the remote device
            LibNet::address::port_t local_port = 0; ///< The port on this device
            LibNet::address::port_t remote_port = 0; ///< The port on the remote device

        public:
            UDPSocket(UserDatagramProtocol* protocol_provider);
            ~UDPSocket();

            void send(uint8_t* data, uint16_t size);
            void close();

            void add_handler(UDPPacketHandler* handler);
            void remove_handler(UDPPacketHandler* handler);

            virtual void handle_udp_payload(uint8_t* data, uint16_t size);
    };

    /**
     * @class UserDatagramProtocol
     * @brief Handles the UDP protocol
     */
    class UserDatagramProtocol : network::IPV4PacketHandler {

        friend class UDPSocket;

        private:
            MaxOS::common::Vector<UDPSocket*> m_connected_sockets; ///< The list of UDP sockets
            static uint16_t m_free_port; ///< The next free port number
            interface::Interface* m_interface = nullptr;

            void handle_new_connection(UDPSocket* socket, LibNet::address::ipv4_address_t remote_ip, LibNet::address::port_t remote_port);


        protected:
            void socket_closed(UDPSocket* socket);

        public:
            UserDatagramProtocol(interface::Interface* interface, network::InternetProtocolVersion4* protocol_backend);
            ~UserDatagramProtocol();

            bool handle_packet(LibNet::address::ipv4_address_t source, LibNet::address::ipv4_address_t destination, uint8_t* internetprotocol_payload, uint32_t size) final;

            void send_udp_packet(UDPSocket* socket, const uint8_t* data, uint16_t size);
            void recv_udp_packet(LibNet::address::ipv4_address_t source, LibNet::address::ipv4_address_t destination, uint8_t* internetprotocol_payload, uint32_t size);

            UDPSocket* connect(const MaxOS::string& address);
            UDPSocket* connect(LibNet::address::ipv4_address_t ip, LibNet::address::port_t port);
            UDPSocket* listen(LibNet::address::port_t port);
    };

}


#endif //NETSERVER_PROTOCOL_TRANSPORT_UDP_H
