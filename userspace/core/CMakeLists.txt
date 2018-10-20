add_executable(init init.c)
add_executable(terminal terminal.c)

install(TARGETS init terminal DESTINATION ${SYSROOT}/sbin)
