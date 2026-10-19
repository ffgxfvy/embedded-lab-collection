ORG 0000H
START:
    MOV R1, #1          
LOOP:
    MOV A, R1
    ADD A, #(TABLE - $ - 3) 
    MOVC A, @A+PC
    MOV P0, A            

    MOV A, R1
    DEC A             
    ANL A, #07H        
    MOV P2, A            

    ACALL DELAY         
    INC R1               
    CJNE R1, #9, LOOP   
    MOV R1, #1           
    SJMP LOOP

DELAY:
    MOV R5, #2
DL1:
    MOV R6, #200
DL2:
    MOV R7, #250
DL3:
    DJNZ R7, DL3
    DJNZ R6, DL2
    DJNZ R5, DL1
    RET

TABLE:
    DB 0FCH, 60H, 0DAH, 0F2H, 66H, 0B6H, 0BEH, 0E0H 
END