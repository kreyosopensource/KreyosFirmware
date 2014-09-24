#ifndef _ANT_DATA_H
#define _ANT_DATA_H
#include "cbsc_rx.h"
#include "hrm_rx.h"

//// Page data fields
union ant_data{
  struct
  {
    CBSCPage0_Data _stCBSCPage0Data;
    CBSCPage0_Data _stCBSCPastPage0Data;

  }cbsc;
  struct
  {
    HRMPage0_Data _stPage0Data;
    HRMPage1_Data _stPage1Data;
    HRMPage2_Data _stPage2Data;
    HRMPage3_Data _stPage3Data;
    HRMPage4_Data _stPage4Data;
  }hrm;
};

extern union ant_data data;
extern UCHAR ucAntChannel;
extern USHORT usDeviceNumber;
extern UCHAR ucTransType;

#define stCBSCPage0Data data.cbsc._stCBSCPage0Data
#define stCBSCPastPage0Data data.cbsc._stCBSCPastPage0Data
#define stPage0Data data.hrm._stPage0Data
#define stPage1Data data.hrm._stPage1Data
#define stPage2Data data.hrm._stPage2Data
#define stPage3Data data.hrm._stPage3Data
#define stPage4Data data.hrm._stPage4Data

#endif