#include <xc.h>
#include "main.h"

extern int min, sec;
//min = 22, sec = 11
void __interrupt() isr(void)
{
    static unsigned int count = 0, sec_count;
    
    if (TMR2IF == 1)
    {
        if (++count == 1250)
        {
            count = 0;
            if(sec > 0) // After every sec.
            {
                sec--;
            }
            else if(sec == 0 && min > 0)
            {
                min--;
                sec = 59;
            }
        }
        
        TMR2IF = 0;
    }
}