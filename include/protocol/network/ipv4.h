/**
 * @file ipv4.h
 * @brief Defines classes and structures for handling Internet Protocol version 4 (IPv4) packets
 *
 * @date 22nd November 2022
 * @author Max Tyson
 */

#ifndef NETSERVER_PROTOCOL_NETWORK_IPV4_H
#define NETSERVER_PROTOCOL_NETWORK_IPV4_H

#include <cstdint>
#include <libnet/protocol/network/ipv4.h>
#include <libnet/address/ipv4.h>
#include <protocol/link/ethernetframe.h>
#include <network/route.h>


namespace NetServer::protocol::network {


    class InternetProtocolVersion4;

	/**
	 * @class IPV4PacketHandler
	 * @brief Handles the payload of a specific IP protocol
	 */
	class IPV4PacketHandler {
		friend class InternetProtocolVersion4;

		protected:
			InternetProtocolVersion4* m_protocol_provider;                  ///< The backend providing the IP protocol
			LibNet::protocol::network::ip_protocol_t m_handled_protocol;    ///< The IP protocol this handler handles

		public:
			IPV4PacketHandler(InternetProtocolVersion4* protocol_provider, LibNet::protocol::network::ip_protocol_t handled_protocol);
			~IPV4PacketHandler();

	        void send(LibNet::address::ipv4_address_t destination, uint8_t* payload_data, uint32_t size);
            virtual bool handle_packet(LibNet::address::ipv4_address_t source, LibNet::address::ipv4_address_t destination, uint8_t* internetprotocol_payload, uint32_t size);
    };

	/**
	 * @class InternetProtocolVersion4
	 * @brief Handles incoming Ethernet Frames and routes them to the appropriate payload handlers
	 */
	class InternetProtocolVersion4 : public link::EthernetFramePacketHandler {

		friend class LibNet::address::IPV4AddressResolver;

	    private:

	        interface::Interface* m_interface;

	        LibNet::address::IPV4AddressResolver* m_resolver = nullptr;
	        NetServer::network::RoutingTable* m_router = nullptr;

			MaxOS::common::Map<LibNet::protocol::network::ip_protocol_t, IPV4PacketHandler*> m_packet_handlers;


		public:
			InternetProtocolVersion4(interface::Interface* interface, link::EthernetFrameProtocol* protocol_provider, LibNet::address::IPV4AddressResolver* resolver, NetServer::network::RoutingTable* router);
			~InternetProtocolVersion4();

			bool handle_packet(uint8_t* ethernetframe_payload, uint32_t size) final;

            void send_ip_packet(LibNet::address::ipv4_address_t dst_ip_be, LibNet::protocol::network::ip_protocol_t protocol, const uint8_t* data, uint32_t size);
            bool recv_ip_packet(uint8_t* data, uint32_t size);

			void add_handler(IPV4PacketHandler* handler);
			void remove_handler(IPV4PacketHandler* handler);

            static uint16_t checksum(const uint16_t* data, uint32_t length_in_bytes);
	};
}


#endif //NETSERVER_PROTOCOL_NETWORK_IPV4_H
