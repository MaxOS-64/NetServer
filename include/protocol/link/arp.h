/**
 * @file arp.h
 * @brief Defines the Address Resolution Protocol (ARP) for resolving IP addresses to MAC addresses
 *
 * @date 11th November 2022
 * @author Max Tyson
 */

#ifndef NETSERVER_PROTOCOL_LINK_ARP_H
#define NETSERVER_PROTOCOL_LINK_ARP_H

#include <cstdint>
#include <libnet/protocol/link/arp.h>
#include <libnet/address/ipv4.h>
#include <protocol/link/arp.h>
#include <protocol/link/ethernetframe.h>


namespace NetServer::protocol::link {

	/**
	 * @class AddressResolutionProtocol
	 * @brief Handles ARP requests and replies
	 */
	class AddressResolutionProtocol : public EthernetFramePacketHandler, public LibNet::address::IPV4AddressResolver {

	    private:

	        interface::Interface* m_interface;

	        void query_address(LibNet::address::ipv4_address_t address) final;

		public:
			AddressResolutionProtocol(interface::Interface* interface, EthernetFrameProtocol* ethernet_frame_provider);
			~AddressResolutionProtocol();

			bool handle_packet(uint8_t* ethernetframe_payload, uint32_t size) final;
	};

}


#endif //NETSERVER_PROTOCOL_LINK_ARP_H
