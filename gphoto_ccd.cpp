#include <sys/time.h>
#include <memory>

#include "gphoto_ccd.h"
#include "GPhoto++.h"

using namespace std;
using namespace GuLinux;
using namespace INDI::Properties;

const int POLLMS           = 500;       /* Polling interval 500 ms */


std::unique_ptr<GPhotoCCD> simpleCCD(new GPhotoCCD());

void ISGetProperties(const char *dev)
{
    simpleCCD->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int num)
{
    simpleCCD->ISNewSwitch(dev, name, states, names, num);
}

void ISNewText(	const char *dev, const char *name, char *texts[], char *names[], int num)
{
    simpleCCD->ISNewText(dev, name, texts, names, num);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int num)
{
    simpleCCD->ISNewNumber(dev, name, values, names, num);
}

void ISNewBLOB (const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
    INDI_UNUSED(dev);
    INDI_UNUSED(name);
    INDI_UNUSED(sizes);
    INDI_UNUSED(blobsizes);
    INDI_UNUSED(blobs);
    INDI_UNUSED(formats);
    INDI_UNUSED(names);
    INDI_UNUSED(n);
}

void ISSnoopDevice (XMLEle *root)
{
    simpleCCD->ISSnoopDevice(root);
}

GPhotoCCD::GPhotoCCD()
{
    logger = make_shared<GPhotoCPP::Logger>([&](const string &m, GPhotoCPP::Logger::Level l) {
        static map<GPhotoCPP::Logger::Level, INDI::Logger::VerbosityLevel> levels {
            {GPhotoCPP::Logger::ERROR, INDI::Logger::DBG_ERROR },
            {GPhotoCPP::Logger::WARNING, INDI::Logger::DBG_WARNING },
            {GPhotoCPP::Logger::INFO, INDI::Logger::DBG_SESSION },
            {GPhotoCPP::Logger::DEBUG, INDI::Logger::DBG_DEBUG },
            {GPhotoCPP::Logger::TRACE, INDI::Logger::DBG_EXTRA_1 },
        };
        DEBUG(levels[l], m.c_str());
    });
    driver = make_shared<GPhotoCPP::Driver>(logger);

    InExposure = false;
}

/**************************************************************************************
** Client is asking us to establish connection to the device
***************************************************************************************/
bool GPhotoCCD::Connect()
{
    try {
        camera =  driver->autodetect();
        if(!camera)
            return false;
    } catch(GPhotoCPP::Exception &e) {
        DEBUG(INDI::Logger::DBG_ERROR, "Can not open camera: Power OK?");
        return false;
    }
    IDMessage(getDeviceName(), "Simple CCD connected successfully!");

    // Let's set a timer that checks teleCCDs status every POLLMS milliseconds.
    SetTimer(POLLMS);

    return true;
}

/**************************************************************************************
** Client is asking us to terminate connection to the device
***************************************************************************************/
bool GPhotoCCD::Disconnect()
{
    camera.reset();
    IDMessage(getDeviceName(), "Simple CCD disconnected successfully!");
    return true;
}

/**************************************************************************************
** INDI is asking us for our default device name
***************************************************************************************/
const char * GPhotoCCD::getDefaultName()
{
    return "GPhoto NG CCD";
}

/**************************************************************************************
** INDI is asking us to init our properties.
***************************************************************************************/
bool GPhotoCCD::initProperties()
{
    // Must init parent properties first!
    INDI::CCD::initProperties();

    PrimaryCCD.setMinMaxStep("CCD_EXPOSURE", "CCD_EXPOSURE_VALUE", 0.001, 3600, 1, false);

    // We set the CCD capabilities
    SetCCDCapability(CCD_CAN_ABORT | CCD_HAS_SHUTTER);

    /* JM 2014-05-20 Make PrimaryCCD.ImagePixelSizeNP writable since we can't know for now the pixel size and bit depth from gphoto */
    PrimaryCCD.getCCDInfo()->p = IP_RW;

    // Add Debug, Simulator, and Configuration controls
    addAuxControls();

    return true;

}

/********************************************************************************************
** INDI is asking us to update the properties because there is a change in CONNECTION status
** This fucntion is called whenever the device is connected or disconnected.
*********************************************************************************************/
bool GPhotoCCD::updateProperties()
{
    // Call parent update properties first
    INDI::CCD::updateProperties();

    if (isConnected()) {
        // Dummy values for now
        SetCCDParams(1280, 1024, 8, 5.4, 5.4);
 	properties[Device].add_switch("ISO", this, {getDeviceName(), "ISO", "ISO", "Image settings"}, ISR_1OFMANY, [&](ISState *states, char **names, int n){ return true; });
	for(auto iso: camera->settings().iso_choices())
	  properties[Device].switch_p("ISO").add(iso, iso, iso == camera->settings().iso() ? ISS_ON : ISS_OFF);
        // Start the timer
        SetTimer(POLLMS);
    } else {
        properties.clear(GPhotoCCD::Device);
    }

    return true;
}


/**************************************************************************************
** Client is asking us to start an exposure
***************************************************************************************/
bool GPhotoCCD::StartExposure(float duration)
{
    ExposureRequest=duration;

    // Since we have only have one CCD with one chip, we set the exposure duration of the primary CCD
    PrimaryCCD.setExposureDuration(duration);

    gettimeofday(&ExpStart,NULL);

    InExposure=true;

    // We're done
    return true;
}

/**************************************************************************************
** Client is asking us to abort an exposure
***************************************************************************************/
bool GPhotoCCD::AbortExposure()
{
    InExposure = false;
    return true;
}
/**************************************************************************************
** How much longer until exposure is done?
***************************************************************************************/
float GPhotoCCD::CalcTimeLeft()
{
    double timesince;
    double timeleft;
    struct timeval now;
    gettimeofday(&now,NULL);

    timesince=(double)(now.tv_sec * 1000.0 + now.tv_usec/1000) - (double)(ExpStart.tv_sec * 1000.0 + ExpStart.tv_usec/1000);
    timesince=timesince/1000;

    timeleft=ExposureRequest-timesince;
    return timeleft;
}

/**************************************************************************************
** Main device loop. We check for exposure and temperature progress here
***************************************************************************************/
void GPhotoCCD::TimerHit()
{
    long timeleft;

    if(isConnected() == false)
        return;  //  No need to reset timer if we are not connected anymore

    if (InExposure)
    {
        timeleft=CalcTimeLeft();

        // Less than a 0.1 second away from exposure completion
        // This is an over simplified timing method, check CCDSimulator and simpleCCD for better timing checks
        if(timeleft < 0.1)
        {
            /* We're done exposing */
            IDMessage(getDeviceName(), "Exposure done, downloading image...");

            // Set exposure left to zero
            PrimaryCCD.setExposureLeft(0);

            // We're no longer exposing...
            InExposure = false;

            /* grab and save image */
            grabImage();

        }
        else
            // Just update time left in client
            PrimaryCCD.setExposureLeft(timeleft);

    }

    SetTimer(POLLMS);
    return;
}

/**************************************************************************************
** Create a random image and return it to client
***************************************************************************************/
void GPhotoCCD::grabImage()
{
    // Let's get a pointer to the frame buffer
    uint8_t * image = PrimaryCCD.getFrameBuffer();

    // Get width and height
    int width = PrimaryCCD.getSubW() / PrimaryCCD.getBinX() * PrimaryCCD.getBPP()/8;
    int height = PrimaryCCD.getSubH() / PrimaryCCD.getBinY();

    // Fill buffer with random pattern
    for (int i=0; i < height ; i++)
        for (int j=0; j < width; j++)
            image[i*width+j] = rand() % 255;

    IDMessage(getDeviceName(), "Download complete.");

    // Let INDI::CCD know we're done filling the image buffer
    ExposureComplete(&PrimaryCCD);
}
