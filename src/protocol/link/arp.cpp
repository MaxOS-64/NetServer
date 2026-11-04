/**
 * @file arp.cpp
 * @brief Implementation of the Address Resolution Protocol (ARP) for resolving IP addresses to MAC addresses
 *
 * @date 11th November 2022
 * @author Max Tyson
 */

#include <protocol/link/arp.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::KPI;
using namespace LibNet;
using namespace LibNet::address;
using namespace LibNet::protocol;
using namespace LibNet::protocol::link;
using namespace NetServer;
using namespace NetServer::interface;
using namespace NetServer::protocol;
using namespace NetServer::protocol::link;


/**
 * @brief Constructs an AddressResolutionProtocol handler.
 *
 * @param protocol_provider The ethernet frame protocol implmentation
 */
AddressResolutionProtocol::AddressResolutionProtocol(Interface* interface, EthernetFrameProtocol* protocol_provider)
: EthernetFramePacketHandler(protocol_provider, EthernetFrameType::ARP),
  m_interface(interface)
{
}

AddressResolutionProtocol::~AddressResolutionProtocol() = default;

/**
 * @brief Called when an ARP packet is received.
 *
 * @param ethernetframe_payload The payload of the ARP packet.
 * @param size The size of the ARP packet.
 * @return True if the device should send a response, false otherwise.
 */
bool AddressResolutionProtocol::handle_packet(uint8_t* ethernetframe_payload, uint32_t size) {

	// Cant  be an ARP packet
	if (size < sizeof(arp_message_t))
		return false;

	auto* arp_message = (arp_message_t*)ethernetframe_payload;

    // Not for ethernet (@todo support others)
    if (arp_message->hardware_type != host_to_network((uint16_t)HardwareType::ETHERNET))
        return false;

    // Not for IPv4 (@todo support others)
    if (arp_message->hardware_type != host_to_network((uint16_t)ProtocolType::IPV4))
        return false;

    // Address is in a unkown format
    if (arp_message->hardware_address_size != MAC_ADDRESS_BYTES || arp_message->protocol_address_size != IP_ADDRESS_BYTES)
        return false;

    // Not for this device
    if (arp_message->dest_ip != host_to_network(m_interface->get_ipv4_address().raw_address()))
        return false;

    switch ((arp_command_t)network_to_host(arp_message->command)) {

        case ARPCommand::REQUEST:
				arp_message->command = host_to_network((uint16_t)ARPCommand::REPLY);

                // Update addressing to be from this device
				arp_message->dest_mac = arp_message->src_mac;
				arp_message->dest_ip = arp_message->src_ip;
				arp_message->src_mac = host_to_network(m_interface->get_mac().raw_address());
				arp_message->src_ip = host_to_network(m_interface->get_ipv4_address().raw_address());
				return true;

		case ARPCommand::REPLY:
            store(network_to_host(arp_message->src_ip), network_to_host(arp_message->src_mac));
			break;

		default:
            klog("Unkown ARP request type 0x%x\n", network_to_host(arp_message->command));
			break;

	}

    // By default, don't send anything back
	return false;

}


/**
 * @brief Request the MAC address of a given IP address.
 *
 * @param address The IP address in BigEndian.
 */
void AddressResolutionProtocol::query_address(ipv4_address_t address) {

	arp_message_t arp_message = { };

	// Set the message metadat
	arp_message.hardware_type = host_to_network((uint16_t)HardwareType::ETHERNET);
	arp_message.protocol = host_to_network((uint16_t)ProtocolType::IPV4);
	arp_message.hardware_address_size = MAC_ADDRESS_BYTES;
	arp_message.protocol_address_size = IP_ADDRESS_BYTES;
	arp_message.command = host_to_network((uint16_t)ARPCommand::REQUEST);

	// Broadcast the address being looked up
    arp_message.src_mac = host_to_network(m_interface->get_mac().raw_address());
    arp_message.src_ip = host_to_network(m_interface->get_ipv4_address().raw_address());
    arp_message.dest_mac = BROADCAST_MAC;
	arp_message.dest_ip = host_to_network(address.raw_address());

	//Send the message
	this->send(arp_message.dest_mac, (uint8_t*)&arp_message, sizeof(arp_message_t));
}

