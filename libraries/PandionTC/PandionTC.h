#ifndef PandionTC_h
#define PandionTC_h

#include "Arduino.h"
#include "AtaLogger.h"

 
class PandionTC {
    public:
      PandionTC();

      void init();
      void setAngle(float angle);
      float currentAngle();

    private:
      AtaLogger ;
};
 
#endif
