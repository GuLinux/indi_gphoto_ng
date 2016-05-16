#ifndef INDI_GPHOTO_CCD_H
#define INDI_GPHOTO_CCD_H


#include <indiccd.h>
#include "indi_properties_map.h"
#include "camera.h"

namespace INDI {
namespace GPhoto {
class GPhotoCCD : public INDI::CCD
{
public:
    GPhotoCCD();

protected:
    // General device functions
    bool Connect();
    bool Disconnect();
    const char *getDefaultName();
    bool initProperties();
    bool updateProperties();

    // CCD specific functions
    bool StartExposure(float duration);
    bool AbortExposure();
    void TimerHit();

private:
    enum PropertiesType { Persistent, Device };
    INDI::Properties::PropertiesMap<PropertiesType> properties;
    Camera::ptr camera;
    
    bool set_iso(const std::vector<ISState> &iso_switches);

    // Utility functions
    float CalcTimeLeft();
    void  grabImage();

    // Are we exposing?
    bool InExposure;
    // Struct to keep timing
    struct timeval ExpStart;

    float ExposureRequest;
    float TemperatureRequest;
    int   timerID;

};
}
}
#endif // INDI_GPHOTO_CCD_H
