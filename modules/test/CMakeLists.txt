add_executable(test.ko test)

install(TARGETS test.ko DESTINATION ${SYSROOT}/lib/modules)
install(TARGETS test.ko DESTINATION ${INITRD}/lib/modules)
