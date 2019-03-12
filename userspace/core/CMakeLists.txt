add_executable(init init.c)
add_executable(sh sh.c)
add_executable(terminal terminal.c)

install(TARGETS init terminal DESTINATION ${SYSROOT}/sbin)
install(TARGETS sh DESTINATION ${SYSROOT}/usr/bin)
