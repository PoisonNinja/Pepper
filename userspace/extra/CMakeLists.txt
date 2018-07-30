add_executable(hello hello.c)
add_executable(signal signal.c)

install(TARGETS hello signal DESTINATION ${SYSROOT}/usr/bin)
