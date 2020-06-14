	.include	"global.s"

	.area _GSINIT

	;ENABLE_RAM_MBC1
	ld      A, #0x0a
	ld	(#0x0000), A
	
	;SET_RAM_MBC1_MODE(1);
	ld      A, #0x01
        ld      (#0x6000), A

	;SWITCH_R0M_MBC1(1);
	ld      A, #0x01
	ld	(.MBC1_ROM_PAGE),a
	ldh	(__current_bank),a
