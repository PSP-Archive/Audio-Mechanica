	.set noreorder

#include "pspstub.s"

	STUB_START "ebootloader",0x40090000,0x00010005
	STUB_FUNC  0xFC04DFAD,eblStartFromMs
	STUB_END
