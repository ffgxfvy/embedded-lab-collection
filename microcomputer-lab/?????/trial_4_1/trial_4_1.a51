ORG 0000H
AJMP MAIN
ORG 0030H

MAIN:
    MOV P1, #0FFH     
    MOV R0, #0       

WAIT:
    JB P3.7, WAIT     
    ACALL DELAY       
    JNB P3.7, $       
    ACALL DELAY       

    INC R0            
    MOV A, R0
    ANL A, #0FH        
    MOV R0, A
    MOV P1, A          

    CJNE R0, #0FH, WAIT 
    MOV P1, #0FFH     
    MOV R0, #0        
    SJMP WAIT

DELAY:                
    MOV R6, #50
DL1:
    MOV R7, #200
DL2:
    DJNZ R7, DL2
    DJNZ R6, DL1
    RET

END