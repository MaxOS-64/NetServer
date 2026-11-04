/**
 * @file icmp.h
 * @brief Defines the Internet Control Message Protocol (ICMP) handler for network communication.
 *
 * @date 24th November 2022
 * @author Max Tyson
 */

#ifndef NETSERVER_PROTOCOL_NETWORK_ICMP_H
#define NETSERVER_PROTOCOL_NETWORK_ICMP_H

#include <cstdint>
#include <libnet/protocol/network/icmp.h>
#include <protocol/network/ipv4.h>

namespace NetServer::protocol::network {

	/**
	 * @class InternetControlMessageProtocol
	 * @brief Handles ICMP packets
	 */
	class InternetControlMessageProtocol : IPV4PacketHandler {

	    private:
			void request_echo_reply(uint32_t ip_be);

	    public:
			InternetControlMessageProtocol(InternetProtocolVersion4* protocol_provider);
			~InternetControlMessageProtocol();

			bool handle_packet(LibNet::address::ipv4_address_t source, LibNet::address::ipv4_address_t destination, uint8_t* internetprotocol_payload, uint32_t size) final;

	        void send_icmp_packet(uint32_t dst_ip_be, uint8_t protocol, const uint8_t* data, uint32_t size);
	        bool recv_icmp_packet(uint8_t* data, uint32_t size);
	};
}


#endif //NETSERVER_PROTOCOL_NETWORK_ICMP_H
