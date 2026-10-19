ORG 0000H
START:
    MOV DPTR, #TABLE      ; ????
LOOP:
    MOV R0, #0            ; ??????0
NEXT:
    MOV A, R0
    MOVC A, @A+DPTR       ; ??????
    MOV P0, A             ; ???P0?
    ACALL DELAY           ; ??
    INC R0                ; ?????
    CJNE R0, #10, NEXT    ; ????10,??
    SJMP LOOP             ; ????

DELAY:
    MOV R5, #50
DL1:
    MOV R6, #200
DL2:
    MOV R7, #200
DL3:
    DJNZ R7, DL3
    DJNZ R6, DL2
    DJNZ R5, DL1
    RET

TABLE:
    DB 0C0H, 0F9H, 0A4H, 0B0H, 99H, 92H, 82H, 0F8H, 80H, 90H  ; ???0~9
END