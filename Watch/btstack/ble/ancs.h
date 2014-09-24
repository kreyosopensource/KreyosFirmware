#ifndef _ANCS_H_
#define _ANCS_H_

typedef enum {
  CategoryIDOther = 0,
  CategoryIDIncomingCall = 1,
  CategoryIDMissedCall = 2,
  CategoryIDVoicemail = 3,
  CategoryIDSocial = 4,
  CategoryIDSchedule = 5,
  CategoryIDEmail = 6,
  CategoryIDNews = 7,
  CategoryIDHealthAndFitness = 8,
  CategoryIDBusinessAndFinance = 9,
  CategoryIDLocation = 10,
  CategoryIDEntertainment = 11,
}CategoryID;

typedef enum {
  EventIDNotificationAdded = 0,
  EventIDNotificationModified = 1,
  EventIDNotificationRemoved = 2,
}EventID;

typedef enum {
  EventFlagSilent = (1 << 0),
  EventFlagImportant = (1 << 1),
}EventFlags;

typedef enum {
  NotificationAttributeIDAppIdentifier = 0,
  NotificationAttributeIDTitle = 1,  //(Needs to be followed by a 2-bytes max length parameter)
  NotificationAttributeIDSubtitle = 2, //(Needs to be followed by a 2-bytes max length parameter)
  NotificationAttributeIDMessage = 3, //(Needs to be followed by a 2-bytes max length parameter)
  NotificationAttributeIDMessageSize = 4,
  NotificationAttributeIDDate = 5,
}NotificationAttributeID;



#endif
