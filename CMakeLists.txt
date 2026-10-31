MAKE_PROG_NOSTD(NetServer src)
TARGET_LINK_LIBRARIES(NetServer.elf net)
TARGET_LINK_LIBRARIES(NetServer.elf driver)