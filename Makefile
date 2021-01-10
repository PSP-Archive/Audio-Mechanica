TARGET = AudioMechanica

PSP_EBOOT_ICON = ICON0.PNG
PSP_EBOOT_PIC1 = PIC1.PNG

OBJS = main.o graphics.o framebuffer.o intraFont.o sceUsbMic.o ebootloader.o

INCDIR = ../include
CFLAGS = -O2 -G0 -Wall 
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = ../lib
LDFLAGS =
LIBS = -lpspusbcam -lpspaudiolib -lpspaudio -lpsppower -lpsphprm -lpspvfpu -lpspgum -lpspgu -lpng -lz -lm -lpsprtc -lpsposk -lpspusb -lpsputility

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Audio Mechanica

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
