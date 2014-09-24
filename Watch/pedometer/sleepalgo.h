/*
 * sleepalgo.h
 *
 *  Created on: Jan 20, 2014
 *  Author: amanda shen
 *  author Texas Instruments, Inc
 * 	version 1.0 -
 * 	note Built with CCS Version 5.5.0.00077
 */

#ifndef SLEEPALGO_H_
#define SLEEPALGO_H_
//#include "stdlib.h"

typedef enum
{
	SLEEP_LEVEL0=0,  //awake
	SLEEP_LEVEL1,	//light sleep
	SLEEP_LEVEL2	//deep sleep
}SLEEP_LEVEL_TYPE;



/*********************************************************************************//**
 * @function name: sleepalgo_init
 * @brief:         Initialize sleep monitor algorithm data structures
 * @param:         unsigned char * pbuffer
 * @return:        unsigned char length
 ***********************************************************************************/
void sleepalgo_init(unsigned char * pbuffer, unsigned char length);

//reset data buffer
void slp_resetdatabuffer(unsigned char * pbuffer);

/*********************************************************************************//**
 * @function name: slp_sample_update
 * @brief:         Update sampling buffer. Should be called every 300ms.
 * @param:         Pointer to x, y, z axis sensor data.
 * @return:        none
 ***********************************************************************************/
unsigned char slp_sample_update(signed char * data_ptr);

/*********************************************************************************//**
 * @function name: slp_status_cal
 * @brief:         calculate the sleep status if sampling buffer is filled銆�
 * @param:         none
 * @return:        none
 ***********************************************************************************/
void slp_status_cal(void);

/*********************************************************************************//**
 * @function name: stop_slp_monitor
 * @brief:         stop monitor.
 * @param:         none
 * @return:        none
 ***********************************************************************************/
void slp_stop_monitor(void);

/*********************************************************************************//**
 * @function name: slp_get_availabledatainfo
 * @brief:         Return the available minutes and lost minutes of sleep monitor.
 *
 * @param:			unsigned int available_minutes,unsigned int lost_minutes
 * @return:        none
 ***********************************************************************************/
void slp_get_availabledatainfo(unsigned int* available_minutes,unsigned int* lost_minutes);
/*********************************************************************************//**
 * @function name: slp_get_classify_time
 * @brief:
 * @param:         sleep level
 * @return:        return the duration of corresponding level
 ***********************************************************************************/
unsigned int slp_get_classify_time(SLEEP_LEVEL_TYPE value);

/*********************************************************************************//**
 * @function name: slp_get_wakeup_times
 * @brief:
 * @param:         none
 * @return:        return the times of wakeup
 ***********************************************************************************/
unsigned char slp_get_wakeup_times(void);

/*********************************************************************************//**
 * @function name: slp_getfallasleep_time
 * @brief:         Return the time of fallasleep.
 * @param:         none
 * @return:
 ***********************************************************************************/
unsigned int slp_getfallasleep_time(void);

unsigned char slp_isbufferfull(void);

/*********************************************************************************//**
 * @function name: slp_skippedsamples
 * @brief:         tell algo how many samples passed. Called when received interrupt
                   from gsensor.(motion detected)
 * @param:         none
 * @return:
 ***********************************************************************************/
void slp_skippedsamples(unsigned int sample_num);
#endif /* SLEEPALGO_H_ */
