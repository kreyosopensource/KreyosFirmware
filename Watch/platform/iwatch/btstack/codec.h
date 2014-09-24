#ifndef _CODEC_H
#define _CODEC_H

extern void codec_setvolume(uint8_t level);
extern uint8_t codec_changevolume(int8_t diff);
extern uint8_t codec_getvolume();
extern void codec_wakeup();
extern void codec_init();
extern void codec_shutdown();
extern void codec_suspend();

extern uint8_t codec_getinput();
extern uint8_t codec_changeinput(int8_t diff);
extern void codec_bypass(int enable);
#endif