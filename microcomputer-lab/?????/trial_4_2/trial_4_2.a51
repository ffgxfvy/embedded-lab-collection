LINE EQU 30H  
ROW EQU 31H  
VAL EQU 32H  

ORG 00H  

START: 
    MOV DPTR, #TABLE         
    MOV P0, #0C0H            

LSCAN: 
    MOV P1, #0FH            

L1: 
    JB P1.0, L2               
    LCALL DELAY              
    JB P1.0, L2             
    MOV LINE, #00H            
    LJMP RSCAN               

L2: 
    JB P1.1, L3               
    LCALL DELAY               
    JB P1.1, L3              
    MOV LINE, #01H           
    LJMP RSCAN               

L3: 
    JB P1.2, L4             
    LCALL DELAY               
    JB P1.2, L4               
    MOV LINE, #02H            
    LJMP RSCAN                

L4: 
    JB P1.3, L1             
    LCALL DELAY              
    JB P1.3, L1               
    MOV LINE, #03H           

RSCAN: 
    MOV P1, #0F0H          

C1: 
    JB P1.4, C2             
    MOV ROW, #00H         
    LJMP CALCU             

C2: 
    JB P1.5, C3             
    MOV ROW, #01H            
    LJMP CALCU               

C3: 
    JB P1.6, C4           
    MOV ROW, #02H          
    LJMP CALCU             

C4: 
    JB P1.7, C1              
    MOV ROW, #03H            

CALCU: 
    MOV A, LINE             
    MOV B, #04H            
    MUL AB                    
    ADD A, ROW              
    MOV VAL, A               
    MOVC A, @A+DPTR          
    MOV P0, A                 
    LJMP LSCAN              

DELAY: 
    MOV R6, #20             
D1: 
    MOV R7, #250             
    DJNZ R7, $               
    DJNZ R6, D1              
    RET                     

TABLE: 
    DB 0C0H, 0F9H, 0A4H, 0B0H, 99H, 92H, 82H, 0F8H
    DB 80H, 90H, 88H, 83H, 0C6H, 0A1H, 86H, 8EH

END