//
// Created by Max Tyson on 01/11/2026.
//

#ifndef LIBNET_INTERFACE_INTERFACE_H
#define LIBNET_INTERFACE_INTERFACE_H

#include <libcommon/vector.h>
#include <libcommon/byte_span.h>

namespace NetServer::interface {

    class Interface;

    class InterfaceBinding {

        private:
            Interface* m_interface_backend;

        public:
            InterfaceBinding();
            ~InterfaceBinding();


            virtual void receive(MaxOS::common::byte_span_t data);
            void send(MaxOS::common::byte_span_t data);
    };

    class Interface {

        private:
            MaxOS::common::Vector<InterfaceBinding*> m_binding;

        public:
            Interface();
            ~Interface();

            void bind(InterfaceBinding* binding);
            void unbind(InterfaceBinding* binding);

            void receive(MaxOS::common::byte_span_t data);
            virtual void send(MaxOS::common::byte_span_t data);
    };

};

#endif //LIBNET_INTERFACE_INTERFACE_H
