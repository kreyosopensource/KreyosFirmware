#include "contiki.h"
#include "btstack/src/hfp.h"

uint8_t ind_values[20];


uint8_t hfp_enable_voicerecog(uint8_t p)
{

}

uint8_t hfp_accept_call(uint8_t c)
{		
}

uint8_t hfp_getstatus(uint8_t ind)
{
	return ind_values[ind];
}

uint8_t codec_getvolume()
{
	return 7;
}

void codec_changevolume(int8_t diff)
{

}

void hfp_test_setstatus(uint8_t ind, uint8_t value)
{
	ind_values[ind] = value;
}

uint8_t hfp_connected()
{
    return 0;
}