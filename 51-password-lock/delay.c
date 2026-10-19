#include "delay.h"
#include <intrins.h>

// ºÚªØŒ¢√Î—” ±
void delay_us(uint us) {
    while(us--) {
        _nop_(); _nop_(); _nop_(); _nop_();
        _nop_(); _nop_(); _nop_(); _nop_();
    }
}

// ºÚªØ∫¡√Î—” ±
void delay_ms(uint ms) {
    uint i;
    while(ms--) {
        for(i = 0; i < 112; i++) {
            _nop_();
        }
    }
}
