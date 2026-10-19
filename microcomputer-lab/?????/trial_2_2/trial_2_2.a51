ORG 0000H
LJMP MAIN
ORG 000BH         
LJMP TIMER0_ISR

SOUND_FLAG EQU 20H.0    
HALF_CYCLE EQU 30H    
CYCLE_COUNT EQU 31H     
DURATION_COUNT EQU 32H 

MAIN:
    MOV TMOD, #01H     
    MOV TH0, #0FEH     
    MOV TL0, #0CH
    MOV HALF_CYCLE, #0
    MOV CYCLE_COUNT, #0
    MOV DURATION_COUNT, #0
    CLR SOUND_FLAG   
    
    SETB TR0          
    SETB ET0          
    SETB EA          
    SJMP $           

TIMER0_ISR:
    CLR TR0            
    MOV TH0, #0FEH    
    MOV TL0, #0CH
    SETB TR0           
    
    JB SOUND_FLAG, SOUND_PHASE
    
SILENCE_PHASE:
    SETB P1.0
    
    INC DURATION_COUNT
    MOV A, DURATION_COUNT
    CJNE A, #250, ISR_END  
    
    MOV DURATION_COUNT, #0
    SETB SOUND_FLAG
    SJMP ISR_END
    
SOUND_PHASE:
    INC HALF_CYCLE
    MOV A, HALF_CYCLE
    CJNE A, #1, CHECK_HALF_CYCLE
    
    CLR P1.0
    SJMP CHECK_DURATION
    
CHECK_HALF_CYCLE:
    CJNE A, #2, CHECK_DURATION
    
    SETB P1.0
    MOV HALF_CYCLE, #0
    INC CYCLE_COUNT
    
CHECK_DURATION:
    MOV A, CYCLE_COUNT
    CJNE A, #125, ISR_END  
    
    MOV CYCLE_COUNT, #0
    MOV HALF_CYCLE, #0
    CLR SOUND_FLAG
    
ISR_END:
    RETI               

END