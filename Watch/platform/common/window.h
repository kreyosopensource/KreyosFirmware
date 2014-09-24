#ifndef _WINDOW_H_
#define _WINDOW_H_
#include "grlib/grlib.h"
#include <stdint.h>

PROCESS_NAME(system_process);

// the active process to handle input and various notifications
#define ui_process (&system_process)

/* Key name for key event */
#define KEY_UP          2
#define KEY_DOWN        3
#define KEY_ENTER       0
#define KEY_EXIT        1

#define KEY_TAP         4
#define KEY_DOUBLETAP   5


// parameter is used as rparam
#define EVENT_TIME_CHANGED            0x90
#define EVENT_NOTIFICATION            0x91
#define EVENT_SPORT_DATA              0x92
#define EVENT_BT_RING                 0x93
#define EVENT_BT_CLIP                 0x94
#define EVENT_BT_CIEV                 0x95
#define EVENT_BT_BVRA                 0x96
#define EVENT_AV                      0x97

// parameter is used as lparam
#define EVENT_WINDOW_CREATED          0xc0
#define EVENT_WINDOW_CLOSING          0xc1
#define EVENT_WINDOW_CLOSED           0xc2
#define EVENT_KEY_PRESSED             0xc3
#define EVENT_KEY_LONGPRESSED         0xc4
#define EVENT_WINDOW_ACTIVE           0xc5
#define EVENT_WINDOW_DEACTIVE         0xc6
#define EVENT_GESTURE_MATCHED         0xc7

// move exit key into another message, so any application want to handle this 
// message, have to be more explict
#define EVENT_EXIT_PRESSED            0xa0
#define EVENT_BT_STATUS               0xa1 // parameters BIT0:ENABLE, BIT1:CONNECT
#define EVENT_ANT_STATUS              0xa2 // parameters BIT0:EnABLE, BIT1:CONNECT
#define EVENT_MPU_STATUS              0xa3 // parameters BIT0:EnABLE
#define EVENT_CODEC_STATUS            0xa4 // parameters BIT0:EnABLE
#define EVENT_WINDOW_PAINT            0xa5 // no parameter
#define EVENT_NOTIFICATION_DONE       0xa6
#define EVENT_NOTIFY_RESULT           0xa7 // the notification result
#define EVENT_FILESYS_LIST_FILE       0xa8 // parameters 0 - end, -1 - error, address - pointer to char*
#define EVENT_STLV_DATA_SENT          0xa9

#define EVENT_FIRMWARE_UPGRADE        0xaa // parameter is rparameter, offset

typedef uint8_t (*windowproc)(uint8_t event, uint16_t lparam, void* rparam);

extern void window_init(uint8_t reason);
extern void window_reload();
extern void window_open(windowproc proc, void* data);
extern void window_invalid(const tRectangle *rect);
extern void status_invalid(void);  // invalid status
extern void window_timer(clock_time_t time);
extern void window_close(void);
extern windowproc window_current();
extern tContext* window_context();
extern void window_postmessage(uint8_t event, uint16_t lparam, void *rparam);

// Common control
extern void window_button(tContext *pContext, uint8_t key, const char* text);
extern void window_progress(tContext *pContext, long lY, uint8_t step);
extern void window_drawtime(tContext *pContext, long y, uint8_t times[3], uint8_t selected);
extern void window_volume(tContext *pContext, long lX, long lY, int total, int current);
extern void window_selecttext(tContext *pContext, const char* pcString, long lLength, long lX, long lY);

#define NOTIFY_OK 0
#define NOTIFY_YESNO 1
#define NOTIFY_ACCEPT_REJECT 2
#define NOTIFY_ALARM  0x10
#define NOTIFY_CONFIRM 0x20

#define NOTIFY_RESULT_OK 1
#define NOTIFY_RESULT_YES 1
#define NOTIFY_RESULT_NO 2
#define NOTIFY_RESULT_ACCEPT 1
#define NOTIFY_RESULT_REJECT 2


extern void window_messagebox(uint8_t icon, const char* message, uint8_t flags);
extern void window_notify_ancs_init();
extern void window_notify(const char* title, const char* message, uint8_t buttons, char icon);
extern void window_notify_ancs(uint8_t command, uint32_t uid, uint8_t flag, uint8_t category);
extern void window_notify_content(const char* title, const char* subtitle, const char* msg, const char* date, uint8_t buttons, char icon);

extern const tRectangle client_clip;
extern const tRectangle fullscreen_clip;

extern uint8_t status_process(uint8_t event, uint16_t lparam, void* rparam);

// Dialogs
extern uint8_t analogclock_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t digitclock_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t menu_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t control_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t countdown_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t watch_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t btconfig_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t configdate_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t configtime_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t configfont_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t stopwatch_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t calendar_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t selftest_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t sportswatch_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t worldclock_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t today_process(uint8_t ev, uint16_t lparam, void* rparam);
extern uint8_t sporttype_process(uint8_t ev, uint16_t lparam, void* rparam);
extern uint8_t phone_process(uint8_t ev, uint16_t lparam, void* rparam);
extern uint8_t script_process(uint8_t ev, uint16_t lparam, void* rparam);
extern uint8_t shutdown_process(uint8_t ev, uint16_t lparam, void* rparam);
extern uint8_t siri_process(uint8_t ev, uint16_t lparam, void* rparam);
extern uint8_t sportwait_process(uint8_t ev, uint16_t lparam, void* rparam);
extern uint8_t configvol_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t configlight_process(uint8_t event, uint16_t lparam, void* rparam);
extern uint8_t upgrade_process(uint8_t ev, uint16_t lparam, void* rparam);
extern uint8_t welcome_process(uint8_t ev, uint16_t lparam, void* rparam);
extern uint8_t about_process(uint8_t ev, uint16_t lparam, void* rparam);
extern uint8_t reset_process(uint8_t ev, uint16_t lparam, void* rparam);
extern uint8_t charging_process(uint8_t ev, uint16_t lparam, void* rparam);
extern uint8_t configalarm_process(uint8_t event, uint16_t lparam, void* rparam);
#if defined(PRODUCT_W002) || defined(PRODUCT_W004)  
extern uint8_t weather_process(uint8_t ev, uint16_t lparam, void* rparam);
extern uint8_t compass_process(uint8_t ev, uint16_t lparam, void* rparam);
#endif
#define MAX_ALARM_COUNT 3
typedef struct _alarm_t
{
  uint8_t flag;
  uint8_t hour;
  uint8_t minutes;
}alarm_t;

#define UI_CONFIG_SIGNATURE 0xFACE0001
typedef struct {
  uint32_t signature;
  uint16_t goal_steps;
  uint16_t goal_distance;
  uint16_t goal_calories;
  uint16_t lap_length;

  // world clock config
  char worldclock_name[6][10];
  int8_t worldclock_offset[6];
  // 66 bytes

  uint8_t default_clock; // 0 - analog, 1 - digit
  // analog clock config
  uint8_t analog_clock;  // num : which clock face
  // digit clock config
  uint8_t digit_clock;   // num : which clock face

  // 69 bytes

  // sports watch config
  uint8_t sports_grid;   // 0 - 3 grid, 1 - 4 grid, 2 - 5 grid
  uint8_t sports_grid_data[5]; // each slot means which grid data to show
  // 75 bytes


  //goals
  // 82 bytes

  uint8_t is_ukuint;
  // 76 bytes

  uint8_t weight; // in kg
  uint8_t height; // in cm
  uint8_t circumference;
  // 85 bytes

  //gesture
  //#define GESTURE_FLAG_ENABLE 0x01
  //#define GESTURE_FLAG_LEFT   0x02
  uint8_t gesture_flag;
  uint8_t gesture_map[4];
  // 90 bytes

  uint8_t volume_level;
  uint8_t light_level;

  //alarms
  alarm_t alarms[MAX_ALARM_COUNT];
  // config 0 - Normal, 1 - Larger, 2 - CJK
  uint8_t font_config;
}ui_config;

extern ui_config* window_readconfig();
extern void window_writeconfig();
extern void window_loadconfig();

// the const strings
extern const char * const week_shortname[];
extern const char * const fontconfig_name[];
extern const char * toEnglish(uint8_t number, char* buffer);
extern const char * toEnglishPeriod(uint32_t seconds, char* buffer);
extern const char* toMonthName(uint8_t month, int shortorlong);
extern const char * PairingWarning;

// #define EVENT_SPORT_DATA              0x92
// lparam defined as below
enum SPORTS_DATA_TYPE
{
    SPORTS_INVALID = -1,
    SPORTS_TIME = 0,
    SPORTS_SPEED_MAX,
    SPORTS_TIME_LAST_GPS,

    SPORTS_SPEED,
    SPORTS_ALT,
    SPORTS_DISTANCE,
    SPORTS_ALT_START,

    SPORTS_HEARTRATE,
    SPORTS_HEARTRATE_AVG,
    SPORTS_HEARTRATE_MAX,
    SPORTS_TIME_LAST_HEARTRATE,

    SPORTS_CADENCE,
    SPORTS_TIME_LAP_BEGIN,
    SPORTS_LAP_BEST,

    SPORTS_STEPS,
    SPORTS_PED_SPEED,
    SPORTS_PED_DISTANCE,
    SPORTS_PED_CALORIES,
    SPORTS_PED_STEPS_START,
    SPORTS_TIME_LAST_PED,

    SPORTS_CALS,

    SPORTS_DATA_MAX
};

//GRID
enum SPORTS_GRID
{
    GRID_WORKOUT = 0,
    GRID_SPEED,
    GRID_HEARTRATE,
    GRID_CALS,

    GRID_DISTANCE,
    GRID_SPEED_AVG,
    GRID_ALTITUTE,
    GRID_TIME,

    GRID_SPEED_TOP,
    GRID_CADENCE,
    GRID_PACE,
    GRID_HEARTRATE_AVG,

    GRID_HEARTRATE_MAX,
    GRID_ELEVATION,
    GRID_LAPTIME_CUR,
    GRID_LAPTIME_BEST,

    GRID_FLOORS,
    GRID_STEPS,
    GRID_PACE_AVG,
    GRID_LAPTIME_AVG,

    GRID_MAX,
};

#endif
