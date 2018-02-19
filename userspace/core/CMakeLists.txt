add_executable(init init.c)

install(TARGETS init DESTINATION ${SYSROOT}/sbin)
