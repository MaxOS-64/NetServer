//
// Created by Max Tyson on 01/11/2026.
//

#include <interface/ethernet.h>

using namespace NetServer;
using namespace NetServer::interface;
using namespace LibDriver;
using namespace LibDriver::generic;

EthernetInterface::EthernetInterface(const MaxOS::string& driver_id)
: EventHandler_Ethernet(driver_id),
  m_ethernet_driver(driver_id)
{
}

EthernetInterface::~EthernetInterface() = default;

void EthernetInterface::send(MaxOS::common::byte_span_t data) {
    m_ethernet_driver.send(data);
}

void EthernetInterface::on_data_received(MaxOS::common::byte_span_t data) {
    receive(data);
}


