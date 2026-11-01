//
// Created by Max Tyson on 01/11/2026.
//

#ifndef LIBNET_INTERFACE_ETHERNET_H
#define LIBNET_INTERFACE_ETHERNET_H

#include <interface/interface.h>
#include <libdriver/generic/ethernet.h>
#include <libdriver/server/ethernet_client.h>
#include <libcommon/string.h>

namespace NetServer::interface {

    class EthernetInterface : public Interface, public LibDriver::generic::EventHandler_Ethernet{

        private:
            LibDriver::generic::EthernetClient m_ethernet_driver;

        public:
            EthernetInterface(const MaxOS::string& driver_id);
            ~EthernetInterface();

            void send(MaxOS::common::byte_span_t data) final;
            void on_data_received(MaxOS::common::byte_span_t data) final;
    };

};


#endif //LIBNET_INTERFACE_ETHERNET_H
