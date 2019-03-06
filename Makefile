TARGET = atomix
OBJS = main.o csprite.o graphics.o framebuffer.o funciones.o

CFLAGS = -O2 -G0 -Wall 
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = -lpspgu -lpsppower -lpng -lz -lm -lstdc++ 
LDFLAGS = 

EXTRA_TARGETS = EBOOT.PBP kxploit
PSP_EBOOT_TITLE = atomix

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
