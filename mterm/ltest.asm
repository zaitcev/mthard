_TEXT Segment byte public 'CODE'
  Assume CS:_TEXT

;
;    Line initator
;    call:    Linit( port, initval_for_BIOS );
;
  Public _Linit
_Linit Proc near
     push DX
     push BP
InitPort = 6
InitVal = 8
     mov BP,SP

     mov DX,InitPort[BP]
     mov AX,InitVal[BP]
     mov AH,000h            ;Initialisation command
     int 014h

     pop  BP
     pop  DX
     ret
_Linit EndP

;
;    Line input tester (a-la dpm())
;    call:    ret = Ltest( port );   ret & 0x100 ==> flag if no char.
;
  Public _Ltest
_Ltest  Proc near
     push DX
     push CX
     push BP
TestFrame Equ 8
     mov BP,SP

     mov AH,003h     ;Get communications status
     mov DX,TestFrame[BP]
     int 014h        ;Call to BIOS line support

     mov CH,001h
     test AH,001h    ;Receiver data ready flag
     jz ret1
     sub CH,CH

     mov AH,002h     ;Receive
     mov DX,TestFrame[BP]
     int 014h        ;Call to BIOS line support
     mov CL,AL
     or  AH,AH
     jnz RecOK
       mov CH,002h   ; Set error flag
     RecOK:

     ret1:
     mov AX,CX
     pop  BP
     pop  CX
     pop  DX
     ret
_Ltest  EndP

;
;    Try to output a character into communications line
;    call:  flag = Ltryout( port, byte );  == 0 if OK.
;
  Public _Ltryout
_Ltryout Proc near
     push DX
     push BP
OutPort = 6
OutChar = 8
     mov BP,SP

     mov DX,OutPort[BP]
     mov AH,003h            ;Get Status Information
     int 014h
     test AH,040h
     mov AX,1               ;Return busy flag
     jz ret2

     mov DX,OutPort[BP]
     mov AX,OutChar[BP]
     mov AH,001h            ;Send character
     int 014h

     sub AX,AX              ;Return OK code

     ret2:
     pop  BP
     pop  DX
     ret
_Ltryout EndP
_TEXT EndS
     End
