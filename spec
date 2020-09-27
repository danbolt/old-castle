/*
	ROM spec file

	Main memory map

  	0x80000000  exception vectors, ...
  	0x80000400  zbuffer (size 320*240*2)
  	0x80025c00  codesegment
	      :  
  	0x8038F800  cfb 16b 3buffer (size 320*240*2*3)

        Copyright (C) 1997-1999, NINTENDO Co,Ltd.
*/

#include <nusys.h>

/* Use all graphic micro codes */
beginseg
	name	"code"
	flags	BOOT OBJECT
	entry 	nuBoot
	address NU_SPEC_BOOT_ADDR
        stack   NU_SPEC_BOOT_STACK
	include "codesegment.o"
	include "$(ROOT)/usr/lib/PR/rspboot.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspL3DEX2.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.Rej.fifo.o"
    include "$(ROOT)/usr/lib/PR/gspF3DEX2.NoN.fifo.o"
    include "$(ROOT)/usr/lib/PR/gspF3DLX2.Rej.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspS2DEX2.fifo.o"
	include "$(ROOT)/usr/lib/PR/aspMain.o"
	include "$(ROOT)/usr/lib/PR/n_aspMain.o"
endseg

beginseg
	name "foyer_dialogues"
	flags OBJECT
	after "code"
	align 32
	include "foyer_dialogues.o"
endseg

beginseg
    name "dm_bank"
    flags RAW
    include "bgm/dm.ctl"
endseg

beginseg
    name "dm_table"
    flags RAW
    include "bgm/dm.tbl"
endseg

beginseg
    name "dm_seq"
    flags RAW
    include "bgm/songs.sbk"  
endseg

beginwave
	name	"nu1"
	include	"code"
	include	"foyer_dialogues"
endwave
