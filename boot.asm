BITS 16
ORG 0x7C00



; sector 1 of floppy, i.e. physical address 0x00007C00 in memory
bootloader_stage_1:
	MOV BX, 0x00		   
	MOV DS, BX

	PUSHA
	MOV AH, 0x02	   ; BIOS int13h "read sector" function
	MOV AL, 4		   ; Number of sectors to read
	MOV CL, 2		   ; start reading at sector 2 of floppy (stage 2 of bootloader)
	MOV CH, 0		   ; Cylinder/track
	MOV DH, 0		   ; Head
	            	   ; DL already contains number of whichever device we are booting from
	MOV BX, 0x00	   ; Segment containing the destination buffer
	MOV ES, BX
	MOV BX, 0x7E00	   ; Destination buffer offset (jump to stage 2)
	INT 0x13
	POPA

  ; check whether A20 was enabled by BIOS, and halt if not (enabling not yet implemented)
  ;     This function is copied from an OSDev wiki article, and it is returning 1, so
  ;     it seems that A20 is enabled. 
  check_a20:
    PUSHF
	PUSH DS
	PUSH ES
	PUSH DI
	PUSH SI
 
	CLI
 
    XOR AX, AX
    MOV ES, AX
    NOT AX
    MOV DS, AX
    MOV DI, 0x0500
    MOV SI, 0x0510
    MOV AL, BYTE [ES:DI]
    PUSH AX
    MOV AL, BYTE [DS:SI]
    PUSH AX
    MOV BYTE [ES:DI], 0x00
    MOV BYTE [DS:SI], 0xFF
    CMP BYTE [ES:DI], 0xFF
    POP AX
    MOV BYTE [DS:SI], AL
    POP AX
    MOV BYTE [ES:DI], AL
    MOV AX, 0
    JE check_a20__exit
    MOV AX, 1
  check_a20__exit:
	CMP AX, 1
	JNE halt 
   
	POP SI
    POP DI
    POP ES
    POP DS
    POPF

	; if A20 is enabled, jump to stage 2 of bootloader
	JMP 0x7E00


  halt:
	HLT


times 510-($-$$) DB 0		; Pad rest of sector
DW 0xAA55			        ; Bootsector signature







; sector 2 of floppy, i.e. physical address 0x00007E00 in memory
bootloader_stage_2:

	; read kernel image from sector 6 of floppy and load it
	;     to an "agreed upon" location
	;     (we should actually read the ELF header to get the
	;     entry point and use that to determine the floppy sector and
	;	  memory location to use ... see elf_read.txt for code)

	PUSHA
	MOV AH, 0x02	   
	MOV AL, 50			; reserve kernel space		   
	MOV CL, 6		   
	MOV CH, 0		   
	MOV DH, 0		   
	            	   
	MOV BX, 0x00	   
	MOV ES, BX
	MOV BX, 0x8400	   
	INT 0x13
	POPA
	
	; install the global descriptor table (flat segmentation)
	CALL installGDT

	; set Protected Enable bit
	CLI
	MOV EAX, CR0
	OR EAX, 1
	MOV CR0, EAX
	STI

	; install Task State Segment for software interrupts
	CALL installTSS

	CLI
	MOV		AX, 0x10	; set data segments to kernel data selector (0x10)	
	MOV		DS, AX
	MOV		SS, AX
	MOV		ES, AX
	MOV		FS, AX
	MOV		GS, AX
	MOV		ESP, 0x00090000		; set stack to grow downwards from address 0x00090000

	; keep running in ring 0
	JMP 0x08:kernel_mode
   

gdt_start: 
	DD 0 				; required null descriptor (offset 0x00)
	DD 0 
  
; kernel code:			; code descriptor (offset 0x08)
	DW 0xFFFF 			; limit low (in paragraphs)
	DW 0x0000 			; base low
	DB 00000000B 		; base middle
	DB 10011010B 		; access
	DB 11001111B 		; granularity
	DB 00000000B 		; base high
 
; kernel data:			; data descriptor (offset 0x10)
	DW 0xFFFF 			; limit low (in paragraphs)
	DW 0x0000 			; base low
	DB 00000000B 		; base middle
	DB 10010010B 		; access
	DB 11001111B 		; granularity
	DB 00000000B		; base high
 
; user code:			; code descriptor (offset 0x18)
    DW 0xFFFF 			; limit low (in paragraphs)
	DW 0x0000 			; base low
	DB 00000000B 		; base middle
	DB 11111010B 		; access
	DB 11001111B 		; granularity
	DB 00000000B		; base high

; user data:			; data descriptor (offset 0x20)
	DW 0xFFFF 			; limit low (in paragraphs)
	DW 0x0000 			; base low
	DB 00000000B 		; base middle
	DB 11110010B 		; access
	DB 11001111B 		; granularity
	DB 00000000B 		; base high

; task state segment for going to ring 0 from ring 3 (offset 0x28)
	DW 0x0000			; limit low (in paragraphs)
	DW 0x7E8C 			; base low
	DB 00000000B 		; base middle
	; access bit 4 must be cleared for TSS descriptor (and other system descriptors)	
	DB 10001001B 		; access
	DB 01001111B 		; granularity
	DB 00000000B 		; base high
	
 
end_of_gdt:
gdtr: 
	DW end_of_gdt - gdt_start - 1 	; limit (Size of GDT)
	DD gdt_start 			        ; base of GDT

installGDT:
	CLI			
	PUSHA
	LGDT [gdtr]	
	STI
	POPA
	RET


installTSS:
	CLI
	PUSHA
	MOV AX, 0x28        ; 0x28, not 0x2B, because RPL = 0
	LTR AX
	STI
	POPA
	RET	 


tss:
	DW 0x0000           ; link
	DW 0x0000           ; reserved
	DD 0x000F0000       ; ESP0
	DW 0x0010           ; SS0
	DW 0x0000           ; reserved
	DD 0x00000000       ; ESP1
	DW 0x0000           ; SS1
	DW 0x0000           ; reserved
	DD 0x00000000       ; ESP2
	DW 0x0000           ; SS2
	DW 0x0000           ; reserved
	DD 0x00000000       ; CR3
	DD 0x00000000       ; EIP
	DD 0x00000000       ; EFLAGS
	DD 0x00000000       ; EAX
	DD 0x00000000       ; ECX
	DD 0x00000000       ; EDX
	DD 0x00000000       ; EBX
	DD 0x00000000       ; ESP
	DD 0x00000000       ; EBP
	DD 0x00000000       ; ESI
	DD 0x00000000       ; EDI
	DW 0x0000           ; ES
	DW 0x0000           ; reserved
	DW 0x0000           ; CS
	DW 0x0000           ; reserved
	DW 0x0000           ; SS
	DW 0x0000           ; reserved
	DW 0x0000           ; DS
	DW 0x0000           ; reserved
	DW 0x0000           ; FS
	DW 0x0000           ; reserved
	DW 0x0000           ; GS
	DW 0x0000           ; reserved
	DW 0x0000           ; LDTR
	DW 0x0000           ; reserved
	DW 0x0000           ; reserved
	DW 0x0000           ; IOPB offset


BITS 32

kernel_mode:
	
	; call into entry point of kernel (i.e. the function kernel_main())
	CALL 0x8400

times 1024-($-$$) DB 0		; Pad rest of sector




; sector 3 of floppy, i.e. physical address 0x00008000 in memory
; CURRENTLY EMPTY



times 1536-($-$$) DB 0


