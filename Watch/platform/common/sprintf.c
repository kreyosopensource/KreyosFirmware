#include "stdarg.h"
#include <stdio.h>
#include <stdint.h>

const unsigned long dv[] = {
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

static const unsigned long hv[] = {
//  4294967296      // 32 bit unsigned max   
   0xFFFFFFFFF,     // +0
    0xFFFFFFFF,     // +1
     0xFFFFFFF,     // +2
      0xFFFFFF,     // +3
       0xFFFFF,     // +4
        0xFFFF,     // +5
         0xFFF,     // +6
          0xFF,     // +7
           0xF,     // +8
             0,     // +9
};


static unsigned int xtobuf(char *output, unsigned long x, const unsigned long *dp)
{
    	char c;
    	unsigned long d;
    	unsigned int len=0;
    	if(x) 
    	{
        	while(x < *dp) ++dp;
        	do {
            		d = *dp++;
            		c = '0';
            		while(x >= d) ++c, x -= d;
            		*output = c;
            		output++;  
            		len++;           		
        	} while(!(d & 1));
    	} 
    	else
    	{	
		*output = '0';
            	output++; 
            	len++;
        }	
        return len;
}

static void puthtobuf(char *output, unsigned n, unsigned char format)
{
	static const char hexl[16] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    	static const char hex[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    	if (format == 'x')
    	{	
    		*output = hexl[n & 15];
      	}	
    	else
    	{	
    		*output = hex[n & 15];
      	}	      	
}

int sprintf(char *output, const char *format, ...)
{
    	char c;
    	int i,x;
    	long n;
    	int len,templen;
        char* tempstr;

    	va_list a;
    	va_start(a, format);
    	while(c = *format++) 
    	{
        	len = 0;
        	if(c == '%') 
        	{
retry:
     			templen=0;
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
                		tempstr = va_arg(a, char*);
				while(*tempstr != '\0')
  				{
  					*output = *tempstr;
    					tempstr++;
    					output++;
  				}
                    		break;
                	
                	case 'c':                       // Char
                		*output=(char)va_arg(a, int);                		
                		output++;                    		
                    		break;
                	
                	case 'd':
                	case 'i':                       // 16 bit Integer
                    		i = va_arg(a, int);
                    		if(i < 0) 
                    		{	
                    			i = -i;
                    			*output = '-';
                    			output++;
            				}                    			
            				for(x=0;x<len-1;x++)
            				{
            					if(i<dv[10-len+x])
            					{
            				    		*output='0';
            				    		output++;	
            					}
            				}
                    		templen = xtobuf(output, (unsigned)i, dv + 5);
                    		output+=templen;
                    		break;
                	
                	case 'u':                       // 16 bit Unsigned
                    		i = va_arg(a, unsigned int);
                    		templen = xtobuf(output, (unsigned)i, dv + 5);
                    		output+=templen;
                    		break;
                	
                	case 'l':                       // 32 bit Long
                    		if (*format == 'u' || *format == 'd')
                    		{
                      			format++;
                      			// fallthrough
                    		}
                	
                	case 'n':                       // 32 bit uNsigned loNg
                    		n = va_arg(a, long);
                    		if(c == 'l' &&  n < 0)
                    		{
                    			n = -n, 
                    			*output = '-';
                    			output++;                    			
                    		}	
                    		templen = xtobuf(output, (unsigned long)n, dv);
                    		output+=templen;
                    		break;
                
                	case 'p':
                    		n = va_arg(a, long);
                    		// 20bit pointer
                    		puthtobuf(output, n >> 16, c); 
                    		output++;
                    		puthtobuf(output, n >> 12, c);
                    		output++;
                    		puthtobuf(output, n >> 8, c);
                    		output++;
                    		puthtobuf(output, n >> 4, c);
                    		output++;
                    		puthtobuf(output, n , c);
                    		output++;
                    		break;
                	
                	case 'X':
                	case 'x':                       // 16 bit heXadecimal
                    		i = va_arg(a, int);
                            for(x=0;x<len-1;x++)
                            {
                                if(i<hv[10-len+x])
                                {
                                        *output='0';
                                        output++;   
                                }
                            }
                    		if (len == 4 || len == 0)
                    		{
                    			puthtobuf(output, i >> 12, c);
                    			output++;
                    			puthtobuf(output, i >> 8, c);                    			
                    			output++;
                    		}
               			puthtobuf(output, i >> 4, c);
               			output++;
               			puthtobuf(output, i , c);                    			                    		
               			output++;
                    		break;
                	
                	case 0: 
                        *output = 0;
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
			*output=c;
                	output++;			
		}
    	}
    	va_end(a);

    	*output = 0;
	return 0;
}