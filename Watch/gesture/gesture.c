/*
* Algorithm is uWave coming from 
*
*/
#include "contiki.h"
#include "window.h"
#include <string.h>
#include <assert.h>

#include "cfs/cfs.h"
#include "gesture.h"
#include <stdio.h>
#include <stdlib.h>
#include <backlight.h>

extern void mpu_switchmode(int mode);

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define MAX_DATAPOINTS 500
// must be 2^n. 1, 2, 4, 8, 16
#define MOVE_WINDOW 4
#define MOVE_STEP 2

static int16_t data[MOVE_WINDOW][3];
static uint8_t datap = 0;

#define NUM_GESTURES 4
#define MAX_GESTURES 35

#include "gesture_sample.h"

static const int8_t *GestureData[NUM_GESTURES * 2] =
{
  LeftGesture1, LeftGesture2, LeftGesture3, LeftGesture4,
  RightGesture1, RightGesture2, RightGesture3, RightGesture4
};

static const int8_t GestureDataLength[NUM_GESTURES * 2] =
{
  sizeof(LeftGesture1)/3, sizeof(LeftGesture2)/3, 
  sizeof(LeftGesture3)/3, sizeof(LeftGesture4)/3,

  sizeof(RightGesture1)/3, sizeof(RightGesture2)/3, 
  sizeof(RightGesture3)/3, sizeof(RightGesture4)/3,
};

CASSERT(sizeof(LeftGesture1) < MAX_GESTURES * 3 * 2, LeftGesture1);
CASSERT(sizeof(LeftGesture2) < MAX_GESTURES * 3 * 2, LeftGesture2);
CASSERT(sizeof(LeftGesture3) < MAX_GESTURES * 3 * 2, LeftGesture3);
CASSERT(sizeof(LeftGesture4) < MAX_GESTURES * 3 * 2, LeftGesture4);

CASSERT(sizeof(RightGesture1) < MAX_GESTURES * 3 * 2, RightGesture1);
CASSERT(sizeof(RightGesture2) < MAX_GESTURES * 3 * 2, RightGesture2);
CASSERT(sizeof(RightGesture3) < MAX_GESTURES * 3 * 2, RightGesture3);
CASSERT(sizeof(RightGesture4) < MAX_GESTURES * 3 * 2, RightGesture4);


static uint16_t *distances;
static uint16_t count;
static enum { STATE_NONE, STATE_RECORDING, STATE_RECON } state;
static uint8_t gestureoffset;

void gesture_init(int8_t _recording, uint8_t leftorright)
{
  // init gesture structure
  distances = (uint16_t*)malloc(NUM_GESTURES * MAX_GESTURES * 2);
  if (!distances)
    return;

  memset(distances, 0, NUM_GESTURES * MAX_GESTURES * 2);
  memset(data, 0, sizeof(data));
  datap = 0;
  count = 0;
  
  if (state != STATE_NONE)
    return;
  
  if (_recording)
  {
    state = STATE_RECORDING;
  }
  else
  {
    state = STATE_RECON;
  }

  if (leftorright)
  {
    gestureoffset = NUM_GESTURES;
  }
  else
  {
    gestureoffset = 0;
  }
  
  mpu_switchmode(1);
}

static uint16_t Dist(const int8_t* p1, const int8_t *p2)
{
  uint16_t t = 
    (p1[0] - p2[0]) * (p1[0] - p2[0])
      + (p1[1] - p2[1]) * (p1[1] - p2[1]) 
        + (p1[2] - p2[2]) * (p1[2] - p2[2]);
  
  return t;
}

#define SCALE_2G (8192 * 2)
#define SCALE_1G (SCALE_2G/2)

static int8_t Normalize(int16_t input)
{
  int16_t value;
  int8_t ret;
  
  if (input == 0)
    return 0;
  else if (input > 0) 
    value = input;
  else
    value = -input;
  
  if (value > SCALE_2G)
  {
    ret = 20;
  }
  else if (value < SCALE_1G)
  {
    ret = (value / 8) * 10 / (SCALE_1G/8);
  }
  else
  {
    ret = 10 + ((value - SCALE_1G)/ 8) * 5/ ((SCALE_1G) / 8);
  }
  
  if (input > 0)
    return ret;
  else
    return -ret;
}

//extern int spp_send(char* buf, int count);
static uint16_t gesture_caculate(int index, const int8_t* point)
{
  const int8_t *currentGesture = GestureData[index + gestureoffset];
  uint8_t gestureLength = GestureDataLength[index + gestureoffset];
  uint16_t *distance = distances + (MAX_GESTURES * index);
  // caculate with template
  uint16_t lastvalue = Dist(&currentGesture[0], point);
  
  if (count == MOVE_WINDOW)
  {
    distance[0] = lastvalue;
    for(int tindex = 1; tindex < gestureLength; tindex++)
    {
      distance[tindex] = distance[tindex - 1] + Dist(&currentGesture[tindex * 3], point);
    }
  }
  else
  {
    if (lastvalue > distance[0])
      lastvalue = distance[0];
    for(int tindex = 1; tindex < gestureLength; tindex++)
    {
      uint16_t local = Dist(&currentGesture[tindex * 3], point);
      uint16_t min = lastvalue;
      if (min > distance[tindex])
        min = distance[tindex];
      if (min > distance[tindex - 1])
        min = distance[tindex - 1];
      if (min > lastvalue)
        min = lastvalue;
      distance[tindex - 1] = lastvalue;
      lastvalue = local + min;
    }
    distance[gestureLength - 1] = lastvalue;
  }
#if 0
  for(int i = 0; i < gestureLength; i++)
  {
    PRINTF("%d ",distance[i]);
  }
  PRINTF("\n  ");
  for(int i = 1; i < gestureLength; i++)
  {
    PRINTF("%d ",distance[i] - distance[i-1]);
  }
  PRINTF("\n");
#endif
  return distance[gestureLength - 1]/(gestureLength + count/MOVE_STEP - 1);
}

void gesture_processdata(int16_t *input)
{
  int8_t result[3];
  
  if (state == STATE_NONE)
    return;
  
  if (state == STATE_RECON && count > MAX_DATAPOINTS)
  {
    PRINTF("No MATCH\n");
    process_post(ui_process, EVENT_GESTURE_MATCHED, (void*)0);
    gesture_shutdown();
    return;
  }
  
  //PRINTF("%d,%d,%d,\n", input[0], input[1], input[2]);
  
  count++;
  
  // integrate into move window average
  // get rid of oldest
  for(int i = 2; i >=0; i--)
  {
    int16_t currentsum;
    data[datap][i] = input[i] / MOVE_WINDOW;        
    if ((datap & (MOVE_STEP - 1)) != (MOVE_STEP -1))
      continue;
    
    currentsum = data[0][i] + data[1][i] + data[2][i] + data[3][i];
    
    //		if (count < MOVE_WINDOW)
    //			continue;
    
    result[i] = Normalize(currentsum);
  }  

  
  datap++;
  datap &= (MOVE_WINDOW - 1);
  if ((datap % MOVE_STEP == 0) && (count >=  MOVE_WINDOW))
  {
    printf("%d,%d,%d,\n", (int)result[0], (int)result[1], (int)result[2]);
    //    PRINTF("%d,%d,%d,\n", (int)result[0], (int)result[1], (int)result[2]);
    if (state == STATE_RECORDING)
    {
      if (count > MAX_DATAPOINTS)
      {
        PRINTF("===\n");
        state = STATE_NONE;
      }
    }
    else
    {
      //PRINTF("%d %d %d\n", result[0], result[1], result[2]);
      uint16_t shortestDistance = 0xffff;
      uint16_t longestDistance = 0;
      uint8_t bestMatch;
      uint32_t totalDistance = 0;
      for(int k = 0; k < NUM_GESTURES; k++)
      {
        uint16_t distance = gesture_caculate(k, result);
        
        PRINTF("%d => %d\t", k, distance);
        if (distance < shortestDistance) {
          shortestDistance = distance;
          bestMatch = k;
        }
        
        if (distance > longestDistance) {
          longestDistance = distance;
        }
        totalDistance += distance;
      }
      
      uint16_t averageDistance = totalDistance/NUM_GESTURES;
      PRINTF("ad: %d, sd: %d, ld: %d, var: %d\n", 
             averageDistance, shortestDistance, longestDistance, longestDistance - shortestDistance);
      
      if ((shortestDistance > averageDistance / 2) || count < MAX_GESTURES)
      {
        PRINTF("almost matched %d\n", bestMatch+1);
        return;
      }
      // matched
      PRINTF("Matched %d\n", bestMatch+1);
      motor_on(200, CLOCK_SECOND/4);
      process_post(ui_process, EVENT_GESTURE_MATCHED, (void*)(bestMatch + 1));
      gesture_shutdown();
      return;
    }
  }
}

void gesture_shutdown()
{
  state = STATE_NONE;
  mpu_switchmode(0);

  if (distances)
  {
    free(distances);
    distances = NULL;
  }
}