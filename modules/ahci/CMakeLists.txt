add_executable(ahci.ko ahci.cc controller.cc port.cc)

install(TARGETS ahci.ko DESTINATION ${SYSROOT}/lib/modules)
install(TARGETS ahci.ko DESTINATION ${INITRD}/lib/modules)

# target_sources(quark.kernel
#     PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/ahci.cc
#     ${CMAKE_CURRENT_SOURCE_DIR}/controller.cc
#     ${CMAKE_CURRENT_SOURCE_DIR}/port.cc
# )