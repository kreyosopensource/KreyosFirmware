#include "contiki.h"
#include "pedometer/sleepalgo.h"

const unsigned char demo_slp_data[60] = {
		0,		0xAA,	0xAA,	0xAA,	0x55,	0xAA,	0xAA,	0xAA,	0xAA,	0xAA,
		0xAA,	0xAA,	0xAA,	0xAA,	0x55,	0xAA,	0xAA,	0xAA,	0xAA,	0xAA,
		0xAA,	0xAA,	0xAA,	0xAA,	0x55,	0xAA,	0xAA,	0xAA,	0xAA,	0xAA,
		0x0,	0xAA,	0xAA,	0xAA,	0x55,	0xAA,	0xAA,	0xAA,	0xAA,	0xAA,
		0xAA,	0xAA,	0xAA,	0xAA,	0x55,	0xAA,	0xAA,	0xAA,	0xAA,	0xAA,
		0xAA,	0xAA,	0xAA,	0xAA,	0x55,	0xAA,	0xAA,	0xAA,	0xAA,	0xAA,
};

void sleepalgo_init(unsigned char * pbuffer, unsigned char length)
{
	memcpy(pbuffer, demo_slp_data, length);
}


//reset data buffer
void Resetdatabuffer(unsigned char * pbuffer)
{}

/*********************************************************************************//**
 * @function name: slp_sample_update
 * @brief:         Update sampling buffer. Should be called every 300ms.
 * @param:         Pointer to x, y, z axis sensor data.
 * @return:        none
 ***********************************************************************************/
unsigned char slp_sample_update(signed char * data_ptr)
{}

/*********************************************************************************//**
 * @function name: slp_status_cal
 * @brief:         calculate the sleep status if sampling buffer is filled。
 * @param:         none
 * @return:        none
 ***********************************************************************************/
void slp_status_cal(void)
{

}

/*********************************************************************************//**
 * @function name: stop_slp_monitor
 * @brief:         stop monitor.
 * @param:         none
 * @return:        none
 ***********************************************************************************/
void slp_stop_monitor(void)
{}

/*********************************************************************************//**
 * @function name: getslpdatainfo
 * @brief:         Return the available minutes and lost minutes of sleep monitor.
 *
 * @param:			unsigned int available_minutes,unsigned int lost_minutes
 * @return:        none
 ***********************************************************************************/
void slp_get_availabledatainfo(unsigned int* available_minutes,unsigned int* lost_minutes)
{
	*available_minutes = 140;
	*lost_minutes = 0;
}
/*********************************************************************************//**
 * @function name: getfallasleep_time
 * @brief:         Return the time of fall asleep
 * @param:         none
 * @return:
 ***********************************************************************************/
unsigned int slp_getfallasleep_time(void)
{
	return 30;
}

/*********************************************************************************//**
 * @function name: getwake_time
 * @brief:         Return the time of awake
 * @param:         none
 * @return:
 ***********************************************************************************/
unsigned char slp_get_wakeup_times(void)
{
	return 4;
}

/*********************************************************************************//**
 * @function name: getsleeping_time
 * @brief:         Return the time of sleeping.
 * @param:         none
 * @return:
 ***********************************************************************************/
unsigned int slp_get_classify_time(SLEEP_LEVEL_TYPE type)
{
	switch(type)
	{
		case SLEEP_LEVEL0:
			return 14;
		case SLEEP_LEVEL1:
			return 500;
		case SLEEP_LEVEL2:
			return 300;
	}
}
