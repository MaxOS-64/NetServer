/**
 * @file ethernetframe.h
 * @brief Defines classes for handling Ethernet Frames and their payloads
 *
 * @date 11th May 2022
 * @author Max Tyson
 *
 * @todo Make big endian helper functions and clairty (ie be_uint16_t etc)
 */

#ifndef NETSERVER_PROTOCOL_LINK_ETHERNETFRAME_H
#define NETSERVER_PROTOCOL_LINK_ETHERNETFRAME_H

#include <cstdint>
#include <libcommon/macros.h>
#include <libcommon/outputStream.h>
#include <libcommon/map.h>
#include <libdriver/generic/ethernet.h>

#include "../../../../../libraries/libcommon/include/libcommon/macros.h"


namespace NetServer::protocol::link {

	/**
	 * @struct EthernetFrameHeader
	 * @brief Structure the raw data. This is the header of an Ethernet Frame
	 *
	 * @typedef ethernet_frame_header_t
	 * @brief Alias for EthernetFrameHeader struct
	 */
	typedef struct PACKED EthernetFrameHeader {

		uint64_t destination_mac : 48;     ///< The mac address of the target (Big Endian)
		uint64_t source_mac : 48;          ///< The mac address of the sender (Big Endian)

		uint16_t type;                  ///< The type (Big Endian) 0x0800 = IPv4, 0x0806 = ARP, 0x86DD = IPv6

	} ethernet_frame_header_t;

	/**
	 * @struct EthernetFrameFooter
	 * @brief Structure the raw data. This is the footer of an Ethernet Frame
	 *
	 * @typedef ethernet_frame_footer_t
	 * @brief Alias for EthernetFrameFooter struct
	 */
	typedef struct PACKED EthernetFrameFooter {

		uint32_t checksum;            ///< checksum of the payload

	} ethernet_frame_footer_t;

	class EthernetFrameHandler;

	/**
	 * @class EthernetFramePayloadHandler
	 * @brief Handles a specific type of Ethernet Frame payload
	 */
	class EthernetFramePayloadHandler {
			friend class EthernetFrameHandler;

		protected:
			EthernetFrameHandler* frame_handler; ///< The Ethernet frame handler this payload handler is connected to
			uint16_t handled_type;               ///< The Ethernet frame type this handler handles

		public:
			EthernetFramePayloadHandler(EthernetFrameHandler* frame_handler, uint16_t handled_type);
			~EthernetFramePayloadHandler();

			virtual bool handle_ethernetframe_payload(uint8_t* ethernetframe_payload, uint32_t size);
			void send(uint64_t destination, uint8_t* data, uint32_t size);
	};


	/**
	 * @class EthernetFrameHandler
	 * @brief Handles incoming Ethernet Frames and routes them to the appropriate payload handlers
	 */
	class EthernetFrameHandler {
		protected:

			MaxOS::common::Map<uint16_t, EthernetFramePayloadHandler*> frame_handlers;  ///< The map of frame handlers by type

			LibDriver::generic::Ethernet* ethernet_driver;                  ///< The driver this frame handler is using
			MaxOS::common::OutputStream* error_messages;                                ///< The output stream for error messages

		public:
			EthernetFrameHandler(LibDriver::generic::Ethernet* driver, MaxOS::common::OutputStream* error_messages);
			~EthernetFrameHandler();

			LibDriver::generic::MediaAccessControlAddress get_mac();
			bool data_received(uint8_t* data, uint32_t size) override;
			void connect_handler(EthernetFramePayloadHandler* handler);
			void send_ethernet_frame(uint64_t destination_mac, uint16_t frame_type, uint8_t* data, uint32_t size);

	};
}


#endif //NETSERVER_PROTOCOL_LINK_ETHERNETFRAME_H
