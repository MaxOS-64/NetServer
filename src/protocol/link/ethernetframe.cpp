/**
 * @file ethernetframe.cpp
 * @brief Implementation of Ethernet Frame Handler and Payload Handler
 *
 * @date 11th May 2022
 * @author Max Tyson
 */

#include <protocol/link/ethernetframe.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::KPI;
using namespace NetServer;
using namespace NetServer::protocol;
using namespace NetServer::protocol::link;
using namespace LibNet;
using namespace LibNet::protocol;
using namespace LibNet::protocol::link;
using namespace LibNet::address;
using namespace LibDriver;
using namespace LibDriver::generic;

/**
 * @brief Construct a new Ether Frame Payload Handler object
 *
 * @param protocol_provider The backend that provides this protocol
 * @param handled_type The type of frame this handler expects
 */
EthernetFramePacketHandler::EthernetFramePacketHandler(EthernetFrameProtocol* protocol_provider, ethernet_frame_type_t handled_type) {

	this->handled_type = handled_type;
	this->protocol_provider = protocol_provider;

    // Register as a packet handler
	protocol_provider->add_handler(this);
}

/**
 * @brief Destroy the EtherFrameHandler:: EtherFrameHandler object, Removes it from the handler list
 */
EthernetFramePacketHandler::~EthernetFramePacketHandler(){
    protocol_provider->remove_handler(this);
};

/**
 * @brief Send an ethernet frame (the packet type is the one this handler marks as handled)
 *
 * @param destination the destination MAC address
 * @param data the data to send
 * @param size the size of the payload
 */
void EthernetFramePacketHandler::send(const mac_t& destination, uint8_t* data, uint32_t size) const {

    protocol_provider->send_ethernet_frame(destination, handled_type, data, size);
}

/**
 * @brief Handle the received ethernet frame payload
 *
 * @param ethernetframe_payload The data received (packet payload)
 * @param size The size of the payload
 *
 * @return True if the data is to be sent back, false otherwise
 */
bool EthernetFramePacketHandler::handle_packet(uint8_t* ethernetframe_payload, uint32_t size) {

	// By default, don't handle it, will be handled in the override
	return false;
}

ethernet_frame_type_t EthernetFramePacketHandler::type() {
    return handled_type;
}

/**
 * @brief Construct a new Ether Frame Handler object
 */
EthernetFrameProtocol::EthernetFrameProtocol(mac_t address)
: m_mac(address)
{

};

EthernetFrameProtocol::~EthernetFrameProtocol() = default;

/**
 * @brief Get the MAC address of this device
 *
 * @return MediaAccessControlAddress The MAC address
 */
mac_t EthernetFrameProtocol::get_mac() {
	return m_mac;
}


/**
 * @brief Send a ethernet frame payload
 *
 * @param destination_mac the destination MAC address
 * @param frame_type the payload protocol
 * @param data the data to send
 * @param size the size of the payload
 */
void EthernetFrameProtocol::send_ethernet_frame(mac_t destination_mac, ethernet_frame_type_t frame_type, uint8_t* data, uint32_t size){
    klog("EFH: Sending frame...\n");

    // Allocate memory for the buffer
    auto* buffer =  new uint8_t[size + sizeof(ethernet_frame_header_t)];
    auto* frame = (ethernet_frame_header_t*) buffer;

    // Set the metadata
    frame->destination_mac  = destination_mac.address();
    frame->source_mac       = m_mac.address();
    frame->type             = host_to_network(frame_type);

    // Copy the payload
    for(uint8_t* src = data + size - 1, * dst = buffer + sizeof(ethernet_frame_header_t) + size - 1; src >= data; --src, --dst)
        *dst = *src;

    // Send the data via the backend
    send({buffer, size + sizeof(ethernet_frame_header_t)});
    klog("EFH: Sent\n");

    delete[] buffer;
}

/**
 * @brief Handle the received packet
 *
 * @param buffer the buffer with the received data
 * @param size the size of the received data
 * @return True if the data is to be sent back, false otherwise
 *
 * @todo Future debugging me: the override is not being called in derived classes
 */
bool EthernetFrameProtocol::recv_ethernet_frame(uint8_t* buffer, uint32_t size) {

	klog("EFH: Data received\n");


	//Check if the size is big enough to contain an ethernet frame
	if(size < sizeof(EthernetFrameHeader))
		return false;

	//Convert to struct for easier use
	auto* frame = (EthernetFrameHeader*) buffer;
	bool send_back = false;

	//Only handle if it is for this device
	if(frame->destination_mac == BROADCAST_MAC || frame->destination_mac == m_mac.address())
	{

		// Find the handler for the protocol
		auto handler_iterator = m_frame_handlers.find((ethernet_frame_type_t)network_to_host(frame->type));

		// No handler found
		if(handler_iterator == m_frame_handlers.end()) {
		    klog("EFH: Unhandled ethernet frame type 0x%x\n", network_to_host(frame->type));
		    return false;
		}

	    //Handle the data
	    send_back = handler_iterator->second->handle_packet(buffer + sizeof(EthernetFrameHeader), size - sizeof(EthernetFrameHeader));
	}

	if(send_back) {

		klog("EFH: Sending back\n");

	    // Update the packt to be directed back to the sender
		frame->destination_mac = frame->source_mac;
		frame->source_mac = m_mac.address();

	}

	return send_back;
}

void EthernetFrameProtocol::receive(byte_span_t data) {

    // Delegate to handler
    bool send_back = recv_ethernet_frame((uint8_t*)data.data, data.size);
    if (send_back)
        send(data);

}

void EthernetFrameProtocol::add_handler(EthernetFramePacketHandler* handler) {

    if (handler == nullptr)
        return;

    m_frame_handlers.insert(handler->type(), handler);
}

void EthernetFrameProtocol::remove_handler(EthernetFramePacketHandler* handler) {
    m_frame_handlers.erase(handler->type());
}
