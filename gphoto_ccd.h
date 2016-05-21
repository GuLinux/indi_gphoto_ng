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
    virtual bool ISNewBLOB(const char* dev, const char* name, int sizes[], int blobsizes[], char* blobs[], char* formats[], char* names[], int n);
    virtual bool ISNewNumber(const char* dev, const char* name, double values[], char* names[], int n);
    virtual bool ISNewText(const char* dev, const char* name, char* texts[], char* names[], int n);
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
    virtual bool saveConfigItems(FILE* fp);

private:
    enum PropertiesType { Persistent, Device };
    INDI::Properties::PropertiesMap<PropertiesType> properties;
    Camera::ptr camera;
    
    // Utility functions

    int   timerID;

};
}
}
#endif // INDI_GPHOTO_CCD_H
