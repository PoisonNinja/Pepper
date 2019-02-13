add_executable(initrd_init init.c)

set_target_properties(initrd_init PROPERTIES OUTPUT_NAME "init")

install(TARGETS initrd_init DESTINATION ${INITRD}/sbin)