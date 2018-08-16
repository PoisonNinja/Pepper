add_executable(ahci.ko ahci.cc controller.cc port.cc)

install(TARGETS ahci.ko DESTINATION ${SYSROOT}/lib/modules)