#ifndef _AVRCP_H_
#define _AVRCP_H_

#include "btstack/utils.h"

/* player attributes */
#define AVRCP_ATTRIBUTE_ILEGAL		0x00
#define AVRCP_ATTRIBUTE_EQUALIZER	0x01
#define AVRCP_ATTRIBUTE_REPEAT_MODE	0x02
#define AVRCP_ATTRIBUTE_SHUFFLE		0x03
#define AVRCP_ATTRIBUTE_SCAN		0x04

/* equalizer values */
#define AVRCP_EQUALIZER_OFF		0x01
#define AVRCP_EQUALIZER_ON		0x02

/* repeat mode values */
#define AVRCP_REPEAT_MODE_OFF		0x01
#define AVRCP_REPEAT_MODE_SINGLE	0x02
#define AVRCP_REPEAT_MODE_ALL		0x03
#define AVRCP_REPEAT_MODE_GROUP		0x04

/* shuffle values */
#define AVRCP_SHUFFLE_OFF		0x01
#define AVRCP_SHUFFLE_ALL		0x02
#define AVRCP_SHUFFLE_GROUP		0x03

/* scan values */
#define AVRCP_SCAN_OFF			0x01
#define AVRCP_SCAN_ALL			0x02
#define AVRCP_SCAN_GROUP		0x03

/* media attributes */
#define AVRCP_MEDIA_ATTRIBUTE_ILLEGAL	0x00
#define AVRCP_MEDIA_ATTRIBUTE_TITLE	0x01
#define AVRCP_MEDIA_ATTRIBUTE_ARTIST	0x02
#define AVRCP_MEDIA_ATTRIBUTE_ALBUM	0x03
#define AVRCP_MEDIA_ATTRIBUTE_TRACK	0x04
#define AVRCP_MEDIA_ATTRIBUTE_N_TRACKS	0x05
#define AVRCP_MEDIA_ATTRIBUTE_GENRE	0x06
#define AVRCP_MEDIA_ATTRIBUTE_DURATION	0x07
#define AVRCP_MEDIA_ATTRIBUTE_LAST	AVRCP_MEDIA_ATTRIBUTE_DURATION

/* play status */
#define AVRCP_PLAY_STATUS_STOPPED	0x00
#define AVRCP_PLAY_STATUS_PLAYING	0x01
#define AVRCP_PLAY_STATUS_PAUSED	0x02
#define AVRCP_PLAY_STATUS_FWD_SEEK	0x03
#define AVRCP_PLAY_STATUS_REV_SEEK	0x04
#define AVRCP_PLAY_STATUS_ERROR		0xFF

/* Notification events */
#define AVRCP_EVENT_STATUS_CHANGED	0x01
#define AVRCP_EVENT_TRACK_CHANGED	0x02
#define AVRCP_EVENT_TRACK_REACHED_END	0x03
#define AVRCP_EVENT_TRACK_REACHED_START	0x04
#define AVRCP_EVENT_PLAYBACK_POS_CHANGED 0x05
#define AVRCP_EVENT_BATT_STATUS_CHANGED 0x06
#define AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED 0x09
#define AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED 0x0a
#define AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED 0x0b

typedef struct _event
{
  uint16_t charset;
  uint16_t len;
  void *data;
}EventAttribute;

extern void avrcp_init();
extern void avrcp_connect(bd_addr_t remote_addr);
extern uint8_t avrcp_connected();
extern void avrcp_disconnect();
extern int avrcp_set_volume(uint8_t volume);
extern int avrcp_enable_notification(uint8_t event);
extern int avrcp_get_attributes(uint32_t item);
extern int avrcp_get_capability();
extern int avrcp_get_playstatus();
extern int avrcp_set_player(uint16_t playerid);

#define EVENT_AV_CONNECTED 0xA0
#define EVENT_AV_DISCONNECTED 0xA1
#define EVENT_AV_STATUS 0xA2
#define EVENT_AV_TITLE  0xA3
#define EVENT_AV_ARTIST 0xA4
#define EVENT_AV_LENGTH 0xA5
#define EVENT_AV_POS    0xA6
#define EVENT_AV_TRACK  0xA7
#endif