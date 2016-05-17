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
    
    virtual bool ISNewSwitch(const char* dev, const char* name, ISState* states, char* names[], int n);

protected:
    // General device functions
    bool Connect();
    bool Disconnect();
    const char *getDefaultName();
    bool initProperties();
    bool updateProperties();

    // CCD specific functions
    bool StartExposure(float duration);
    void TimerHit();

private:
    enum PropertiesType { Persistent, Device };
    INDI::Properties::PropertiesMap<PropertiesType> properties;
    Camera::ptr camera;
    
    // Utility functions
    void  grabImage();

    int   timerID;

};
}
}
#endif // INDI_GPHOTO_CCD_H
