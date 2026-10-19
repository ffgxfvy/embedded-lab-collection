ORG 0000H
LJMP MAIN
ORG 000BH          
LJMP TIMER0_ISR

MAIN:
    MOV TMOD, #01H 
    MOV TH0, #0FCH 
    MOV TL0, #18H
    SETB TR0      
    SETB ET0      
    SETB EA       
    SJMP $         

TIMER0_ISR:
    CLR TR0        
    MOV TH0, #0FCH 
    MOV TL0, #18H
    CPL P1.5      
    SETB TR0       
    RETI           

END