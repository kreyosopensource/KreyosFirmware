#ifndef _ANT_H
#define _ANT_H

typedef enum {
  MODE_PAIR = 0,
  MODE_HRM = 1,
  MODE_CBSC = 2
}ModeEnum;

void ant_init(ModeEnum mode);
void ant_shutdown(void);
#endif