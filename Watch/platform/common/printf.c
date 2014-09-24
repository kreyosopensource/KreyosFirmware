#include "stdarg.h"
#include <stdio.h>
#include <stdint.h>
extern const unsigned long dv[];
/*
static const unsigned long dv[] = {
//  4294967296      // 32 bit unsigned max
    1000000000,     // +0
     100000000,     // +1
      10000000,     // +2
       1000000,     // +3
        100000,     // +4
//       65535      // 16 bit unsigned max
         10000,     // +5
          1000,     // +6
           100,     // +7
            10,     // +8
             1,     // +9
};
*/
int puts(const char* s)
{
  	while(*s != '\0')
  	{
    		putchar(*s);
    		s++;
  	}

  	return 0;
}

static void xtoa(unsigned long x, const unsigned long *dp)
{
    	char c;
    	unsigned long d;
    	if(x) 
    	{
        	while(x < *dp) ++dp;
        	do {
            		d = *dp++;
            		c = '0';
            		while(x >= d) ++c, x -= d;
            		putchar(c);
        	} while(!(d & 1));
    	} 
    	else
        	putchar('0');
}

static void puth(unsigned n, unsigned char format)
{
	static const char hexl[16] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    	static const char hex[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    	if (format == 'x')
      		putchar(hexl[n & 15]);
    	else
      		putchar(hex[n & 15]);
}

int printf(const char *format, ...)
{
    	char c;
    	int i;
    	long n;
    	int len;

#ifdef MSP430        
    	if (!uartattached)
        	return 0;
#endif
    	va_list a;
    	va_start(a, format);
    	while(c = *format++) 
    	{
        	len = 0;
        	if(c == '%') 
        	{
retry:     			
            		switch(c = *format++) 
            		{
                	case '0':
                    		if (*format >= '0' || *format <= '9')
                    		{
                        		len = *format - '0';
                        		format++;
                        		goto retry;
                    		}
                	
                	case 's':                       // String
                    		puts(va_arg(a, char*));
                    		break;
                	
                	case 'c':                       // Char
                		putchar((char)va_arg(a, int));                    		
                    		break;
                	
                	case 'd':
                	case 'i':                       // 16 bit Integer
                    		i = va_arg(a, int);
                    		if(i < 0) i = -i, putchar('-');
                    		xtoa((unsigned)i, dv + 5);
                    		break;
                	
                	case 'u':                       // 16 bit Unsigned
                    		i = va_arg(a, unsigned int);
                    		xtoa((unsigned)i, dv + 5);
                    		break;
                	
                	case 'l':                       // 32 bit Long
                    		if (*format == 'u' || *format == 'd')
                    		{
                      			format++;
                      			// fallthrough
                    		}
                	
                	case 'n':                       // 32 bit uNsigned loNg
                    		n = va_arg(a, long);
                    		if(c == 'l' &&  n < 0) n = -n, putchar('-');
                    		xtoa((unsigned long)n, dv);
                    		break;
                
                	case 'p':
                    		n = va_arg(a, long);
                    		// 20bit pointer
                    		puth(n >> 16, c);
                    		puth(n >> 12, c);
                    		puth(n >> 8, c);
                    		puth(n >> 4, c);
                    		puth(n, c);
                    		break;
                	
                	case 'X':
                	case 'x':                       // 16 bit heXadecimal
                    		i = va_arg(a, int);
                    		if (len == 4 || len == 0)
                    		{
                      			puth(i >> 12, c);
                      			puth(i >> 8, c);
                    		}
                    		puth(i >> 4, c);
                    		puth(i, c);
                    		break;
                	
                	case 0: 
                    		return 0;
                	default: 
                    		if (c > '0' && c <= '9')
                    		{
                        		len = c - '0';
                        		goto retry;
                    		}
                		goto bad_fmt;
            		}
        	} 
        	else
        	{	
bad_fmt:    
			putchar(c);
		}
    	}
    	va_end(a);

	return 0;
}