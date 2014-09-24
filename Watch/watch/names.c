#include "contiki.h"
#include "window.h"
#include <string.h>
#include <stdio.h>

static const char * const month_name[] =
{
  "January", "February", "March", "April", "May",
  "June", "July", "August", "September", "October",
  "November", "December"
};

static const char* const english_name[] =
{
  "ZERO", "ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT",
  "NINE", "TEN", "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN",
  "FIFTEEN", "SIXTEEN", "SEVENTEEN", "EIGHTEEN", "NINETEEN"
};

static const char* const english_name_prefix[] =
{
    "TWENTY", "THIRTY", "FORTY", "FIFTY", "SIXTY",
    "SEVENTY", "EIGHTY", "NINTY"
};

static const char * const month_shortname[] = {
  "JAN","FEB","MAR","APR", "MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"
};

const char* const week_shortname[] = {
  "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"
};

const char* const fontconfig_name[] = {
  "Normal",
  "Large",
  "International",
};

// Only support number less than 100
const char* toEnglish(uint8_t number, char* buffer)
{
  if (number < 20)
  {
    return english_name[number];
  }

  // larger than 20
  strcpy(buffer, english_name_prefix[(number / 10) - 2]);
  if (number % 10 != 0)
  {
    strcat(buffer, " ");
    strcat(buffer, english_name[number % 10]);
  }

  return buffer;
}

const char* toEnglishPeriod(uint32_t seconds, char* buffer)
{
  if (seconds < 60)
  {
    return "now";
  }

  if (seconds < 120)
  {
    return "1m ago";
  }

  if (seconds < 3600)
  {
    // how many minutes
    sprintf(buffer, "%dm ago", seconds/60);
    return buffer;
  }

  if (seconds < 3600 * 24)
  {
    sprintf(buffer, "%dh ago", seconds/3600);
    return buffer;
  }

  if (seconds < 3600 * 24 * 5) // 5days max
  {
    sprintf(buffer, "%dd ago", seconds/(3600*24));
    return buffer;    
  }

  return "a long while ago";
}

// shortorlong short = 0, long = 1
const char* toMonthName(uint8_t month, int shortorlong)
{
  if (month >= 12 || month == 0)
    month = 1;
  if (shortorlong)
    return month_name[month - 1];
  else
    return month_shortname[month - 1];
}

const char* PairingWarning = "Please Pair your smartphone to the Meteor.";
