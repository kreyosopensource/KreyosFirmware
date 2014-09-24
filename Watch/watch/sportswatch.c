
#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "window.h"
#include "grlib/grlib.h"
#include "memlcd.h"
#include "platform/iwatch/rtc.h"
#include "ant/ant.h"
#include "stlv.h"
#include "stlv_client.h"
#include "ble_handler.h"

#include "cfs/cfs.h"
#include "btstack/include/btstack/utils.h"
#include "pedometer/pedometer.h"
#include "sportsdata.h"
#include "status.h"

#define GRID_3 			0
#define GRID_4 			1
#define GRID_5 			2

#define GPS_TIMEOUT     30

// The basic data to generate others
static uint32_t base_data[SPORTS_DATA_MAX] = {0};

// the data for each grid
static uint32_t grid_data[5];
static uint32_t workout_time;
// configuration for sport watch
// in ui_config
//  grid type
//  grid1, grid2, grid3, grid4, grid5
//    grid0 is always time spent

#define FORMAT_NONE 0
#define FORMAT_TIME 1
#define FORMAT_SPD  2
#define FORMAT_DIST 3
#define FORMAT_CALS 4
#define FORMAT_ALT  5
#define FORMAT_PACE 6

struct _datapoints
{
  const char *name; // use \t to seperate the string
  const char *unit_uk;
  const char *unit;
  const uint8_t format;
};

static const struct _datapoints datapoints[] =
{
  {"Total Workout Time", NULL,  NULL,   FORMAT_NONE},
  {"Speed",              "mph", "km/h", FORMAT_SPD},
  {"Heart Rate",         "bpm", "bpm",  FORMAT_NONE},
  {"Calories",           "cal", "KJ",   FORMAT_CALS},
  {"Distance",           "mile","km",   FORMAT_DIST},
  {"Avg Speed",          "mph", "km/p", FORMAT_NONE},
  {"Altitude",           "ft",  "mt",   FORMAT_ALT},
  {"Time of the Day",    NULL,  NULL,   FORMAT_TIME},
  {"Top Speed",          "mph", "km/h", FORMAT_SPD},
  {"Cadence",            "cpm", "cpm",  FORMAT_NONE},
  {"Pace",               "min", "min",  FORMAT_PACE},
  {"Avg. Heart Rate",    "bpm", "bpm",  FORMAT_NONE},
  {"Top Heart Rate",     "bpm", "bpm",  FORMAT_NONE},
  {"Elevation Gain",     "ft",  "mt",   FORMAT_ALT},
  {"Current Lap",        "min", "min",  FORMAT_PACE},
  {"Best Lap",           "min", "min",  FORMAT_PACE},
  {"Floors",             NULL,  NULL,   FORMAT_NONE},
  {"Steps",              NULL,  NULL,   FORMAT_NONE},
  {"Avg. Pace",          "min", "min",  FORMAT_PACE},
  {"Avg. Lap",           "min", "min",  FORMAT_PACE},
};

static const tRectangle region_3grid[] =
{
  {0, 0, LCD_WIDTH, 83},
  {0, 84, 74, LCD_Y_SIZE},
  {75, 85, LCD_WIDTH, LCD_Y_SIZE}
};


static const tRectangle region_4grid[] =
{
  {0, 0, LCD_WIDTH, 44},
  {0, 45, LCD_WIDTH, 84},
  {0, 85, LCD_WIDTH, 124},
  {0, 125, LCD_WIDTH, LCD_Y_SIZE},
};

static const tRectangle region_5grid[] =
{
  {0, 0, LCD_WIDTH, 44},
  {0, 45, 72, 104},
  {72, 45, LCD_WIDTH, 104},
  {0, 105, 72, LCD_Y_SIZE},
  {72, 105, LCD_WIDTH, LCD_Y_SIZE},
};

static const tRectangle *regions[] =
{
  region_3grid, region_4grid, region_5grid
};

static const int upload_data_interval = 3;
static const int save_data_interval = 60;

static const uint8_t s_meta_running[] = {DATA_COL_STEP, DATA_COL_CALS, DATA_COL_DIST, DATA_COL_HR};
static const uint8_t s_meta_biking[]  = {DATA_COL_CADN, DATA_COL_CALS, DATA_COL_DIST, DATA_COL_HR};

// Find which grid slot contains the specific data
static int findDataGrid(uint8_t data)
{
  uint8_t totalgrid = window_readconfig()->sports_grid + 3;

  for(uint8_t g = 0; g < totalgrid; g++)
  {
    if (window_readconfig()->sports_grid_data[g] == data)
      return g;
  }

  return -1;
}

static void drawGridTime(tContext *pContext)
{
  char buf[20];
  uint8_t time[3];
  time[0] = workout_time % 60;
  time[1] = (workout_time / 60 ) % 60;
  time[2] = workout_time / 3600;

  GrContextForegroundSet(pContext, ClrBlack);
  switch(window_readconfig()->sports_grid)
  {
  case GRID_3:
    GrContextFontSet(pContext, (tFont*)&g_sFontExIcon16);
    GrStringDraw(pContext, "i", 1, 10, 15, 0);
    GrContextFontSet(pContext, &g_sFontGothic18);
    GrStringDraw(pContext, datapoints[0].name, -1, 28, 15, 0);
    GrContextFontSet(pContext, &g_sFontGothic28b);
    sprintf(buf, "%02d:%02d:%02d", time[2], time[1], time[0]);
    GrStringDrawCentered(pContext, buf, -1, LCD_WIDTH/2, 50, 0);
    break;
  case GRID_4:
  case GRID_5:
    GrContextFontSet(pContext, &g_sFontGothic28b);
    sprintf(buf, "%02d:%02d:%02d", time[2], time[1], time[0]);
    GrStringDrawCentered(pContext, buf, -1, LCD_WIDTH/2, 18, 0);
    break;
  }
}

// draw one grid data
static void drawGridData(tContext *pContext, uint8_t grid, uint32_t data)
{
  int width;
  if (grid == 0)
  {
    // draw time
    drawGridTime(pContext);
    return;
  }

  int index = findDataGrid(grid);
  if (index == -1)
  {
    index = 4; // default to distance
  }
  struct _datapoints const *d = &datapoints[grid];

  char buf[20];
  uint32_t tmp = data;
  const char* unit = window_readconfig()->is_ukuint ? d->unit_uk : d->unit;
  switch(d->format)
  {
    case FORMAT_TIME:
      sprintf(buf, "%02u:%02u", (uint16_t)(data / 60), (uint16_t)(data % 60));
      break;
    case FORMAT_SPD:
      tmp = data * 36 / 10; //to km per hour
      if (window_readconfig()->is_ukuint)
        tmp = tmp * 621 / 1000; // to mile
      sprintf(buf, "%u.%01u", (uint16_t)(tmp / 100), (uint16_t)(tmp % 100));
      break;
    case FORMAT_DIST:
      tmp = tmp / 10;
      if (window_readconfig()->is_ukuint)
        tmp = tmp * 621 / 1000; // to mile
      if (tmp >= 1000)
      {
        sprintf(buf, "%u.%02u", (uint16_t)(tmp / 1000), (uint16_t)(tmp % 1000));
      }
      else
      {
        if (window_readconfig()->is_ukuint)
        {
            tmp = data * 82 / 250;
            unit = "ft";
        }
        else
        {
            unit = "mt";
        }
        sprintf(buf, "%03u",  (uint16_t)(tmp % 1000));
      }

      break;
    case FORMAT_CALS: //kcal
      if (!window_readconfig()->is_ukuint)
        tmp = tmp * 21 / 5; //to KJ
      if (tmp >= 1000)
        sprintf(buf, "%u", (uint16_t)(tmp / 1000));
      else
        sprintf(buf, "0.%02u", (uint16_t)(tmp % 1000));

      break;
    case FORMAT_ALT:
      if (window_readconfig()->is_ukuint)
          tmp = tmp * 82 / 25;
      sprintf(buf, "%u.%01u", (uint16_t)(tmp / 100), (uint16_t)(tmp % 100));
      break;
    case FORMAT_PACE:
      if (window_readconfig()->is_ukuint)
          tmp = tmp * 621 / 1000;
      sprintf(buf, "%u.%01u", (uint16_t)(tmp / 60), (uint16_t)((tmp % 60) * 5 / 3));
      break;
    default:
      sprintf(buf, "%d", (uint16_t)data);
      break;
  }

  GrContextForegroundSet(pContext, ClrWhite);
  // other generic grid data
  switch(window_readconfig()->sports_grid)
  {
  case GRID_3:
    GrContextFontSet(pContext, &g_sFontGothic18);
    GrStringDraw(pContext, d->name, -1,  region_3grid[index].sXMin + 8, region_3grid[index].sYMin, 0);
    GrContextFontSet(pContext, &g_sFontGothic28b);
    GrStringDraw(pContext, buf, -1, region_3grid[index].sXMin + 8, region_3grid[index].sYMin + 20, 0);
    if (unit)
    {
      GrContextFontSet(pContext, &g_sFontGothic18);
      GrStringDraw(pContext, unit, -1, region_3grid[index].sXMin + 8, region_3grid[index].sYMin + 50, 0);
    }
    break;
  case GRID_4:
    GrContextFontSet(pContext, &g_sFontGothic18);
    GrStringDrawWrap(pContext, d->name, region_4grid[index].sXMin + 8, region_4grid[index].sYMin + 8,
                    (region_4grid[index].sXMax - region_4grid[index].sXMin) / 3, 1);

    GrContextFontSet(pContext, &g_sFontGothic28b);
    width = GrStringWidthGet(pContext, buf, -1);
    GrStringDraw(pContext, buf, -1, (region_4grid[index].sXMax - region_4grid[index].sXMin) * 3 / 4 - width,
      region_4grid[index].sYMin + 10, 0);
    if (unit)
    {
      GrContextFontSet(pContext, &g_sFontGothic18);
      GrStringDraw(pContext, unit, -1, (region_4grid[index].sXMax - region_4grid[index].sXMin) * 3 / 4  + 8, region_4grid[index].sYMin + 20, 0);
    }
    break;
  case GRID_5:
    GrContextFontSet(pContext, &g_sFontGothic18);
    GrStringDraw(pContext, d->name, -1,  region_5grid[index].sXMin + 8, region_5grid[index].sYMin, 0);
    GrContextFontSet(pContext, &g_sFontGothic28b);
    width = GrStringWidthGet(pContext, buf, -1);
    GrStringDraw(pContext, buf, -1, region_5grid[index].sXMax - 4 - width, region_5grid[index].sYMin + 15, 0);
    if (unit)
    {
      GrContextFontSet(pContext, &g_sFontGothic18);
      GrStringDraw(pContext, unit, -1, region_5grid[index].sXMin + 8, region_5grid[index].sYMin + 40, 0);
    }
    break;
  }
}

static void onDraw(tContext *pContext)
{
  // first draw the datetime
  uint8_t totalgrid = window_readconfig()->sports_grid + 3;

  GrContextForegroundSet(pContext, ClrWhite);
  switch(window_readconfig()->sports_grid)
  {
  case GRID_3:
    GrRectFill(pContext, &region_3grid[0]);
    GrLineDrawV(pContext, region_3grid[1].sXMax, region_3grid[1].sYMin, region_3grid[1].sYMax);
    break;
  case GRID_4:
    GrRectFill(pContext, &region_4grid[0]);
    GrLineDrawH(pContext, region_4grid[1].sXMin, region_4grid[1].sXMax, region_4grid[1].sYMax);
    GrLineDrawH(pContext, region_4grid[2].sXMin, region_4grid[2].sXMax, region_4grid[2].sYMax);
    break;
  case GRID_5:
    GrRectFill(pContext, &region_5grid[0]);
    GrLineDrawH(pContext, region_5grid[1].sXMin, region_5grid[2].sXMax, region_5grid[1].sYMax);
    GrLineDrawV(pContext, region_5grid[1].sXMax, region_5grid[1].sYMin, region_5grid[3].sYMax);
    break;
  }

  GrContextForegroundSet(pContext, ClrWhite);
  for(uint8_t g = 0; g < totalgrid; g++)
  {
    drawGridData(pContext, window_readconfig()->sports_grid_data[g] , grid_data[g]);
  }
}

const static uint32_t shiftval = 1;

static void updateDistance(uint32_t value, uint32_t* gris_mask)
{
    (*gris_mask) |= (shiftval << GRID_SPEED_AVG);
    (*gris_mask) |= (shiftval << GRID_PACE_AVG);
    (*gris_mask) |= (shiftval << GRID_CALS);

    uint16_t lap_len = window_readconfig()->lap_length * 100;
    if (lap_len > 50)
    {
      uint32_t oldval = base_data[SPORTS_DISTANCE] % lap_len;
      uint32_t newval = value % lap_len;
      if (oldval > newval)
      {
          uint32_t lap_time = workout_time - base_data[SPORTS_TIME_LAP_BEGIN];
          if (lap_time < base_data[SPORTS_LAP_BEST])
          {
              base_data[SPORTS_LAP_BEST] = lap_time;
              (*gris_mask) |= (shiftval << GRID_LAPTIME_BEST);
          }
          base_data[SPORTS_TIME_LAP_BEGIN] = workout_time;
      }
    }
}

static void updateHeartRate(uint32_t value, uint32_t* gris_mask)
{
    base_data[SPORTS_TIME_LAST_HEARTRATE] = workout_time;

    if (value > base_data[SPORTS_HEARTRATE_MAX])
    {
        base_data[SPORTS_HEARTRATE_MAX] = value;
        (*gris_mask) |= (shiftval << GRID_HEARTRATE_MAX);
    }

    if (workout_time > 0)
    {
        base_data[SPORTS_HEARTRATE_AVG] = (
          base_data[SPORTS_HEARTRATE_AVG] * base_data[SPORTS_TIME_LAST_HEARTRATE] +
          value * (workout_time - base_data[SPORTS_TIME_LAST_HEARTRATE])
          ) / workout_time;
        (*gris_mask) |= (shiftval << GRID_HEARTRATE_AVG);
    }

}

static void updateSpeed(uint32_t value, uint32_t* gris_mask)
{
    if (value > base_data[SPORTS_SPEED_MAX])
    {
        base_data[SPORTS_SPEED_MAX] = value;
        (*gris_mask) |= (shiftval << GRID_SPEED_TOP);
    }

    uint32_t cals = value * window_readconfig()->weight * 9 / 2;
    base_data[SPORTS_CALS] += (cals * (workout_time - base_data[SPORTS_TIME_LAST_GPS]) / 1000);
    (*gris_mask) |= (shiftval << GRID_CALS);
    (*gris_mask) |= (shiftval << GRID_PACE);
}

static void updateAlt(uint32_t value, uint32_t* gris_mask)
{
    if (base_data[SPORTS_ALT_START] > (uint32_t)1000000)
        base_data[SPORTS_ALT_START] = value;
}

void updatePedSpeed(uint32_t value, uint32_t* gris_mask)
{
    if (base_data[SPORTS_SPEED] == 0 ||
        base_data[SPORTS_TIME_LAST_GPS] == 0 ||
        workout_time - base_data[SPORTS_TIME_LAST_GPS] > 10)
    {
        *gris_mask |= (shiftval << GRID_SPEED);
    }
}

void updatePedDist(uint32_t value, uint32_t* gris_mask)
{
    if (base_data[SPORTS_TIME_LAST_GPS] == 0 ||
        workout_time - base_data[SPORTS_TIME_LAST_GPS] > 10)
    {
        *gris_mask |= (shiftval << GRID_DISTANCE);
        if (value > base_data[SPORTS_PED_DISTANCE])
            base_data[SPORTS_DISTANCE] += (value - base_data[SPORTS_PED_DISTANCE]);
        else
            base_data[SPORTS_DISTANCE] += value;
    }
}

static uint32_t updateBaseData(uint8_t datatype, uint32_t value)
{
  if (datatype >= SPORTS_DATA_MAX)
      return 0;

//  if (datatype != 0)
//    printf("updateBaseData(%u, %lu)\n", datatype, value);

  uint32_t grids_mask = 0;
  switch (datatype)
  {
      case SPORTS_TIME:
          grids_mask |= (shiftval << GRID_WORKOUT);
          base_data[datatype] = value; //the raw value must be updated at last
          break;

      case SPORTS_DISTANCE:
          updateDistance(value, &grids_mask);
          grids_mask |= (shiftval << GRID_DISTANCE);
          base_data[SPORTS_TIME_LAST_GPS] = workout_time;
          base_data[datatype] += value; //the raw value must be updated at last
          break;

      case SPORTS_HEARTRATE:
          updateHeartRate(value, &grids_mask);
          grids_mask |= (shiftval << GRID_HEARTRATE);
          base_data[datatype] = value; //the raw value must be updated at last
          break;

      case SPORTS_SPEED:
          if (value != 0xffffffff)
          {
               updateSpeed(value, &grids_mask);
               grids_mask |= (shiftval << GRID_SPEED);
               base_data[SPORTS_TIME_LAST_GPS] = workout_time;
               base_data[datatype] = value; //the raw value must be updated at last
          }
          break;

      case SPORTS_ALT:
          if (value != 0xffffffff)
          {
              updateAlt(value, &grids_mask);
              grids_mask |= (shiftval << GRID_ALTITUTE);

              base_data[SPORTS_TIME_LAST_GPS] = workout_time;
              base_data[datatype] = value; //the raw value must be updated at last
          }
          break;

      case SPORTS_STEPS:
          if (base_data[datatype] == 0)
          {
              base_data[SPORTS_PED_STEPS_START] = value;
              base_data[datatype] = 1; //the raw value must be updated at last
          }
          else
          {
              base_data[datatype] = value - base_data[SPORTS_PED_STEPS_START]; //the raw value must be updated at last
          }
          grids_mask |= (shiftval << GRID_STEPS);
          break;

      case SPORTS_PED_SPEED:
          updatePedSpeed(value, &grids_mask);
          base_data[datatype] = value; //the raw value must be updated at last
          break;

      case SPORTS_PED_DISTANCE:
          updatePedDist(value, &grids_mask);
          base_data[datatype] = value; //the raw value must be updated at last
          break;

      case SPORTS_PED_CALORIES:
          if (base_data[datatype] != 0)
          {
            if (value > base_data[datatype])
              base_data[SPORTS_CALS] += (value - base_data[datatype]);
            else
              base_data[SPORTS_CALS] += value;
            base_data[datatype] = value;
          }
          grids_mask |= (shiftval << GRID_CALS);
          break;

      default:
          base_data[datatype] = value; //the raw value must be updated at last
          break;
  }
  return grids_mask;
}

static uint32_t getGridDataItem(uint8_t slot)
{
    static uint8_t s_grid_data_map[] = {
        SPORTS_TIME,
        SPORTS_SPEED,
        SPORTS_HEARTRATE,
        SPORTS_DATA_MAX, //GRID_CALS
        SPORTS_DISTANCE,
        SPORTS_DATA_MAX, //GRID_SPEED_AVG
        SPORTS_ALT,
        SPORTS_DATA_MAX, //GRID_TIME
        SPORTS_SPEED_MAX,
        SPORTS_DATA_MAX, //GRID_CADENCE
        SPORTS_DATA_MAX, //GRID_PACE
        SPORTS_HEARTRATE_AVG,
        SPORTS_HEARTRATE_MAX,
        SPORTS_DATA_MAX, //GRID_ELEVATION
        SPORTS_DATA_MAX, //GRID_LAPTIME_CUR
        SPORTS_LAP_BEST,
        SPORTS_DATA_MAX, //GRID_FLOORS
        SPORTS_STEPS,
        SPORTS_DATA_MAX, //GRID_PACE_AVG
        SPORTS_DATA_MAX, //GRID_LAPTIME_AVG
    };

    uint8_t sport_data_id = s_grid_data_map[slot];
    if (base_data[SPORTS_TIME_LAST_GPS] == 0 ||
        workout_time - base_data[SPORTS_TIME_LAST_GPS] > 10)
    {
        if (sport_data_id == SPORTS_SPEED)
            return base_data[SPORTS_PED_SPEED];
    }

    if (sport_data_id != SPORTS_DATA_MAX)
    {
        return base_data[sport_data_id];
    }

    switch (slot)
    {
        case GRID_CALS:
            return base_data[SPORTS_CALS];
            break;

        case GRID_SPEED_AVG:
            if (workout_time > 0)
                return base_data[SPORTS_DISTANCE] * 10 / workout_time;
            break;


        case GRID_PACE:
            if (base_data[SPORTS_SPEED] * 60 > (uint32_t)(100000 / 60))
                return 100000 / base_data[SPORTS_SPEED];
            break;

        case GRID_ELEVATION:
            if (base_data[SPORTS_ALT_START] < (uint32_t)1000000)
                return base_data[SPORTS_ALT] - base_data[SPORTS_ALT_START];
            break;

        case GRID_LAPTIME_CUR:
            if (base_data[SPORTS_SPEED] * 60 > (window_readconfig()->lap_length * 100 / 60))
                return window_readconfig()->lap_length * 100 / base_data[SPORTS_SPEED];
            break;

        case GRID_PACE_AVG:
            {
                if (workout_time == 0) return 0;
                uint32_t avgSpd = base_data[SPORTS_DISTANCE] * 10 / workout_time;
                if (avgSpd * 60 > 100000 / 60)
                    return 100000 * avgSpd;
            }
            break;

        case GRID_LAPTIME_AVG:
            {
                if (workout_time == 0) return 0;
                uint32_t avgSpd = base_data[SPORTS_DISTANCE] * 10 / workout_time;
                if (avgSpd * 60 > 100000 / 60)
                    return window_readconfig()->lap_length * 100 / avgSpd;
            }
            break;

        case GRID_TIME:
        case GRID_CADENCE:
        case GRID_FLOORS:
            break;
    }
    return 0;
}

static void updateGridData(uint32_t mask)
{
    ui_config* config = window_readconfig();

    for (int j = 0; j < GRID_MAX; ++j)
    {
        if ((mask & (shiftval << j)) == 0)
            continue;

        int slot = findDataGrid(j);
        if (slot != -1)
        {
            grid_data[slot] = getGridDataItem(j);
            //if (j != 0)
            //    printf("updateGridData(id=%s):%u\n", datapoints[j].name, (uint16_t)grid_data[slot]);
            window_invalid(&regions[config->sports_grid][slot]);
        }
    }
}

static void updateData(uint8_t datatype, uint32_t value)
{
    uint32_t grid_mask = updateBaseData(datatype, value);
    updateGridData(grid_mask);
}

static uint8_t sportnum;
static uint8_t sports_type = 0;
static uint32_t s_sports_data[4] = {0};

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define _PROC_VALUE(type, index) \
    data[index] = base_data[type] >= s_sports_data[index] ? base_data[type] - s_sports_data[index] : 0

static void saveSportsData()
{
  uint8_t hour, minute, second;
  rtc_readtime(&hour, &minute, &second); 

  uint32_t data[4] = {0};
  const uint8_t* meta = NULL;
  if (get_mode() == DATA_MODE_RUNNING)
  {
    meta = s_meta_running;
    _PROC_VALUE(SPORTS_STEPS,    0);
    _PROC_VALUE(SPORTS_CALS,     1);
    _PROC_VALUE(SPORTS_DISTANCE, 2);
    data[3] = base_data[SPORTS_HEARTRATE];
  }
  else if (get_mode() == DATA_MODE_BIKING)
  {
    meta = s_meta_biking;
    _PROC_VALUE(SPORTS_CADENCE,  0);
    _PROC_VALUE(SPORTS_CALS,     1);
    _PROC_VALUE(SPORTS_DISTANCE, 2);
    data[3] = base_data[SPORTS_HEARTRATE];
  }

  if (data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 0)
  {
    printf("no valid data\n");
    return;
  }

  printf("save sports data\n");
  write_data_line(get_mode(), hour, minute, meta, data, sizeof(data) / sizeof(data[0]));

  for (int i = 0; i < sizeof(data) / sizeof(data[0]); ++i)
  {
    s_sports_data[i] += data[i];
  }
}

void cleanUpSportsWatchData()
{
  for (int i = 0; i < (int)(sizeof(grid_data)/sizeof(grid_data[0])); i++)
    grid_data[i] = 0;

  for (int i = 0; i < (int)(sizeof(base_data)/sizeof(base_data[0])); i++)
    base_data[i] = 0;

  base_data[SPORTS_ALT_START] = 2000000; //set to height no possible on earth to indicate invalid data

  memset(&s_sports_data, 0, sizeof(s_sports_data));

  workout_time = 0;

}


uint8_t sportswatch_process(uint8_t event, uint16_t lparam, void* rparam)
{
  UNUSED_VAR(lparam);
  switch(event)
  {
  case EVENT_WINDOW_CREATED:
    {
      if (rparam == (void*)0)
      {
          //running
          sports_type = SPORTS_DATA_FLAG_RUN;
          set_mode(DATA_MODE_RUNNING);
        //ant_init(MODE_HRM);
      }
      else
      {
          //cycling
          sports_type = SPORTS_DATA_FLAG_BIKE;
          set_mode(DATA_MODE_BIKING);
        //ant_init(MODE_CBSC);
      }
      rtc_enablechange(SECOND_CHANGE);

      cleanUpSportsWatchData();

      ui_config* config = window_readconfig();
      sportnum = config->sports_grid + 4;

      add_watch_status(WS_SPORTS);

      ble_start_sync(2, get_mode());
      return 0x80; // disable status
    }
  case EVENT_SPORT_DATA:
    {
      //printf("got a sport data \n");
      updateData(lparam, (uint32_t)rparam);
      break;
    }
  case EVENT_TIME_CHANGED:
    {
      workout_time++;
      updateData(SPORTS_TIME, workout_time);

      if (upload_data_interval > 0 &&
          workout_time % upload_data_interval == 0)
      {
        ui_config* config = window_readconfig();
        sportnum = config->sports_grid + 4;

        //STLV over RFCOMM
        send_sports_data(0, 
          sports_type | SPORTS_DATA_FLAG_START, 
          config->sports_grid_data, grid_data, sportnum);

        //BLE
        ble_send_sports_data(grid_data, 5);

      }

      if (workout_time % save_data_interval == 0 &&
       (get_mode() & DATA_MODE_PAUSED) != 0)
      {
        saveSportsData();
      }

      break;
    }
  case EVENT_WINDOW_PAINT:
    {
      tContext *pContext = (tContext*)rparam;
      GrContextForegroundSet(pContext, ClrBlack);
      GrRectFill(pContext, &fullscreen_clip);
      GrContextForegroundSet(pContext, ClrWhite);
      onDraw(pContext);
      break;
    }
  case EVENT_WINDOW_CLOSING:
    {
      rtc_enablechange(0);
#if PRODUCT_W001
      ant_shutdown();
#endif
      
      uint8_t dummy_stlv_meta = 0;
      uint32_t dummy_stlv_data = 0;
      send_sports_data(0, sports_type | SPORTS_DATA_FLAG_STOP, &dummy_stlv_meta, &dummy_stlv_data, 1);

      saveSportsData();
      ble_stop_sync();
      set_mode(DATA_MODE_NORMAL);
      del_watch_status(WS_SPORTS);
      break;
    }
  default:
    return 0;
  }

  return 1;
}

