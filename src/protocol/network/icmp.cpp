/**
 * @file icmp.cpp
 * @brief Implementation of the Internet Control Message Protocol (ICMP) handler
 *
 * @date 24th November 2022
 * @author Max Tyson
 */

#include <protocol/network/icmp.h>


using namespace  MaxOS;
using namespace  MaxOS::common;
using namespace  MaxOS::KPI;
using namespace  LibNet;
using namespace  LibNet::protocol;
using namespace  LibNet::protocol::network;
using namespace  LibNet::address;
using namespace  NetServer;
using namespace  NetServer::protocol;
using namespace  NetServer::protocol::network;

/**
 * @brief Constructs an InternetControlMessageProtocol handler.
 *
 * @param internet_protocol_handler The Internet protocol handler to use.
 * @param error_messages The output stream to use for error messages.
 */
InternetControlMessageProtocol::InternetControlMessageProtocol(InternetProtocolVersion4* protocol_provider)
: IPV4PacketHandler(protocol_provider, ICMP)
{
}

InternetControlMessageProtocol::~InternetControlMessageProtocol() = default;

/**
 * @brief Called by the InternetProtocolProvider when a new packet has arrived
 *
 * @param source  The source IP address of the packet
 * @param destination  The destination IP address of the packet
 * @param payload_data  The payload of the packet
 * @param size The size of the payload

 * @return True if the packet is to be sent back to the sender, false otherwise
 *
 * @todo Reply to ping requests
 * @todo Implement the rest of the ICMP messages
 */
bool InternetControlMessageProtocol::handle_packet(ipv4_address_t source, ipv4_address_t destination, uint8_t *payload_data, uint32_t size)
{

    klog("ICMP received a packet\n");

    // Cannot be a icmp packet
    if(size < sizeof(icmp_header_t))
        return false;

    auto* icmp = (icmp_header_t*)payload_data;

    switch ((ICMPType)icmp->type) {

        case ICMPType::ECHO_REPLY:
            break;

        case ICMPType::ECHO_REQUEST:

            // Create a response
            icmp -> type = (uint8_t)ICMPType::ECHO_REPLY;
            icmp -> checksum = 0;
            icmp -> checksum = InternetProtocolVersion4::checksum((uint16_t*) &icmp, sizeof(icmp_header_t));

            // Send data back
            return true;
    }

    return false;
}

/**
 * @brief Sends an ICMP echo request to the specified IP address
 *
 * @param ip_be The IP address to send the request to
 */
void InternetControlMessageProtocol::request_echo_reply(uint32_t ip_be) {

    klog("ICMP: Sending echo request\n");

    icmp_header_t icmp = {};
    icmp.type = (uint8_t)ICMPType::ECHO_REQUEST;
    icmp.code = 0;
    icmp.checksum = 0;
    icmp.data = 0x69420;
    icmp.checksum = InternetProtocolVersion4::checksum((uint16_t*)&icmp, sizeof(icmp_header_t));

	send(ip_be, (uint8_t*) &icmp, sizeof(icmp_header_t));
    klog("ICMP: Echo request sent\n");
}
