#################
# Configuration #
#################

# Supported architectures
# x86      - 32 bit x86
# x86_64   - 64 bit x86
ARCH ?= x86_64

####################################
# DO NOT TOUCH ANYTHING BELOW THIS #
####################################
ifeq ($(ARCH),x86)
    ARCH := i386
endif

