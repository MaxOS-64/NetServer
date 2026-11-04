//
// Created by Max Tyson on 01/11/2026.
//

#ifndef LIBNET_INTERFACE_INTERFACE_H
#define LIBNET_INTERFACE_INTERFACE_H

#include <libcommon/vector.h>
#include <libcommon/byte_span.h>
#include <libnet/address/mac.h>
#include <libnet/address/ipv4.h>

namespace NetServer::interface {

    class Interface;

    class InterfaceBinding {

        protected:
            Interface* m_interface;

        public:
            InterfaceBinding(Interface* interface);
            ~InterfaceBinding();


            virtual void handle_packet(MaxOS::common::byte_span_t data);
            void send(MaxOS::common::byte_span_t data);
    };

    class Interface {

        private:
            MaxOS::common::Vector<InterfaceBinding*> m_binding;

            LibNet::address::mac_t m_mac_address;
            LibNet::address::ipv4_address_t m_ipv4_address;
            LibNet::address::Subnet m_subnet_mask;

        public:
            Interface();
            ~Interface();

            void bind(InterfaceBinding* binding);
            void unbind(InterfaceBinding* binding);

            void receive(MaxOS::common::byte_span_t data);
            virtual void send(MaxOS::common::byte_span_t data);

            LibNet::address::mac_t get_mac() const;
            LibNet::address::ipv4_address_t get_ipv4_address() const;

            void set_ipv4_address(LibNet::address::ipv4_address_t address);
    };

};

#endif //LIBNET_INTERFACE_INTERFACE_H
