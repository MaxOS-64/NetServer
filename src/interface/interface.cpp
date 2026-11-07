//
// Created by Max Tyson on 01/11/2026.
//

#include <interface/interface.h>


using namespace NetServer;
using namespace NetServer::interface;

InterfaceBinding::InterfaceBinding(Interface* interface)
: m_interface(interface)
{
}

InterfaceBinding::~InterfaceBinding() = default;

void InterfaceBinding::handle_packet(MaxOS::common::byte_span_t data) {

    NOT_IMPLEMENTED;

}

void InterfaceBinding::send(MaxOS::common::byte_span_t data) {
    m_interface->send(data);
}

Interface::Interface() {
}

Interface::~Interface() = default;


void Interface::set_ipv4_address(LibNet::address::ipv4_address_t address) {
    m_ipv4_address = address;
}

void Interface::set_mac_address(LibNet::address::mac_t mac) {
    m_mac_address = mac;
}

void Interface::bind(InterfaceBinding* binding) {
    m_binding.push_back(binding);
}

void Interface::unbind(InterfaceBinding* binding) {
    m_binding.erase(binding);
}

void Interface::receive(MaxOS::common::byte_span_t data) {

    for (const auto& binding : m_binding)
        binding->handle_packet(data);

}

void Interface::send(MaxOS::common::byte_span_t data) {

    NOT_IMPLEMENTED;

}

LibNet::address::mac_t Interface::get_mac() const {

    return m_mac_address;

}

LibNet::address::ipv4_address_t Interface::get_ipv4_address() const {
    return m_ipv4_address;
}


