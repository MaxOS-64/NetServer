/**
 * @file ethernetframe.h
 * @brief Defines classes for handling Ethernet Frames and their payloads
 *
 * @date 11th May 2022
 * @author Max Tyson
 *
 */

#ifndef NETSERVER_PROTOCOL_LINK_ETHERNETFRAME_H
#define NETSERVER_PROTOCOL_LINK_ETHERNETFRAME_H

#include <cstdint>
#include <libcommon/macros.h>
#include <libcommon/outputStream.h>
#include <libcommon/map.h>
#include <libdriver/generic/ethernet.h>
#include <libnet/byteorder.h>
#include <libnet/address/mac.h>
#include <libnet/protocol/link/ethernetframe.h>
#include <libdriver/server/ethernet_client.h>
#include <interface/interface.h>


namespace NetServer::protocol::link {


	class EthernetFrameProtocol;

	/**
	 * @class EthernetFramePacketHandler
	 * @brief Handles a specific type of Ethernet Frame payload
	 */
	class EthernetFramePacketHandler {
		friend class EthernetFrameProtocol;

		protected:
			EthernetFrameProtocol* protocol_provider;                       ///< The backend that provides this protocol
			LibNet::protocol::link::ethernet_frame_type_t handled_type;     ///< The type of frame this handler expects

		public:
			EthernetFramePacketHandler(EthernetFrameProtocol* protocol_provider, LibNet::protocol::link::ethernet_frame_type_t handled_type);
			~EthernetFramePacketHandler();

            void send(const LibNet::address::mac_t& destination, uint8_t* data, uint32_t size) const;
            virtual bool handle_packet(uint8_t* ethernetframe_payload, uint32_t size);

	        LibNet::protocol::link::ethernet_frame_type_t type();
    };


	/**
	 * @class EthernetFrameProtocol
	 * @brief Handles incoming Ethernet Frames and routes them to the appropriate payload handlers
	 */
	class EthernetFrameProtocol : public interface::InterfaceBinding{
		protected:

			MaxOS::common::Map<LibNet::protocol::link::ethernet_frame_type_t, EthernetFramePacketHandler*> m_frame_handlers;  ///< The map of frame handlers by type

	        LibNet::address::mac_t m_mac;

		public:
			EthernetFrameProtocol(LibNet::address::mac_t mac);
			~EthernetFrameProtocol();

			LibNet::address::mac_t get_mac();

            void send_ethernet_frame(LibNet::address::mac_t destination, LibNet::protocol::link::ethernet_frame_type_t frame_type, uint8_t* data, uint32_t size);
	        bool recv_ethernet_frame(uint8_t* data, uint32_t size);

	        void receive(MaxOS::common::byte_span_t data) final;

            void add_handler(EthernetFramePacketHandler* handler);
            void remove_handler(EthernetFramePacketHandler* handler);
    };
}


#endif //NETSERVER_PROTOCOL_LINK_ETHERNETFRAME_H
