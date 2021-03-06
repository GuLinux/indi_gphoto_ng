#include <sys/time.h>
#include <memory>

#include "gphoto_ccd.h"
#include "c++/containers_streams.h"
#include "realcamera.h"
#include "simulationcamera.h"

using namespace std;
using namespace GuLinux;
using namespace INDI::Properties;
using namespace INDI::GPhoto;
const int POLLMS           = 500;       /* Polling interval 500 ms */


std::unique_ptr<GPhotoCCD> gphotoCCD(new GPhotoCCD());

void ISGetProperties(const char *dev)
{
    gphotoCCD->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int num)
{
    gphotoCCD->ISNewSwitch(dev, name, states, names, num);
}

void ISNewText(	const char *dev, const char *name, char *texts[], char *names[], int num)
{
    gphotoCCD->ISNewText(dev, name, texts, names, num);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int num)
{
    gphotoCCD->ISNewNumber(dev, name, values, names, num);
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
    gphotoCCD->ISSnoopDevice(root);
}

GPhotoCCD::GPhotoCCD() : log {this, "GPhotoCCD"}
{
}

/**************************************************************************************
** Client is asking us to establish connection to the device
***************************************************************************************/
bool GPhotoCCD::Connect()
{
    DEBUG(INDI::Logger::DBG_DEBUG, __PRETTY_FUNCTION__);
    try {
camera =  isSimulation() ? Camera::ptr {new SimulationCamera{this}} :
        Camera::ptr {new RealCamera{this}};
    } catch(std::exception &e) {
        log.error() << e.what();
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
    DEBUG(INDI::Logger::DBG_DEBUG, __PRETTY_FUNCTION__);
    if(isSimulation())
        return true;

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
    DEBUG(INDI::Logger::DBG_DEBUG, __PRETTY_FUNCTION__);

    PrimaryCCD.setMinMaxStep("CCD_EXPOSURE", "CCD_EXPOSURE_VALUE", 0.001, 3600, 1, false);

    // We set the CCD capabilities
    SetCCDCapability(CCD_HAS_SHUTTER);

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
    DEBUG(INDI::Logger::DBG_DEBUG, __PRETTY_FUNCTION__);

    if (isConnected()) {
        // Dummy values for now
        SetCCDParams(1280, 1024, 8, 5.4, 5.4);
        try {
            camera->setup_properties(properties[Device]);
            properties[Device].add_switch("ISO", this, {getDeviceName(), "ISO", "ISO", "Image Settings"}, ISR_1OFMANY, [&](const vector<Switch::UpdateArgs> &states) {
                auto on_switch = make_stream(states).first(Switch::On);
                return on_switch && camera->set_iso(get<1>(*on_switch));
            });

            for(auto iso: camera->available_iso() )
                properties[Device].switch_p("ISO").add(iso, iso, iso==camera->current_iso() ? ISS_ON : ISS_OFF);

            properties[Device].add_switch("FORMAT", this, {getDeviceName(), "FORMAT", "FORMAT", "Image Settings"}, ISR_1OFMANY, [&](const vector<Switch::UpdateArgs> &states) {
                auto on_switch = make_stream(states).first(Switch::On);
                return on_switch && camera->set_format(get<1>(*on_switch));
            });

            for(auto iso: camera->available_formats() )
                properties[Device].switch_p("FORMAT").add(iso, iso, iso==camera->current_format() ? ISS_ON : ISS_OFF);
            // Start the timer
            properties[Device].register_unregistered_properties();
        } catch(std::exception &e) {
            log.error() << e.what();
            return false;
        }
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
    try {
        if(camera->shoot_status().status != Camera::ShootStatus::Idle || ! camera->shoot(Camera::Seconds {duration}))
            return false;
        // Since we have only have one CCD with one chip, we set the exposure duration of the primary CCD
        PrimaryCCD.setExposureDuration(duration);
    } catch(std::exception &e) {
        log.error() << e.what();
        return false;
    }

    // We're done
    return true;
}

/**************************************************************************************
** Main device loop. We check for exposure and temperature progress here
***************************************************************************************/
void GPhotoCCD::TimerHit()
{
    if(isConnected() == false)
        return;  //  No need to reset timer if we are not connected anymore

    auto shoot_status = camera->shoot_status();
    if (shoot_status.status == Camera::ShootStatus::Idle)
        PrimaryCCD.setExposureLeft(camera->shoot_status().remaining.count());
    if (shoot_status.status == Camera::ShootStatus::Finished) {
        IDMessage(getDeviceName(), "Exposure done, downloading image...");
        // Set exposure left to zero
        PrimaryCCD.setExposureLeft(0);
        if(camera->write_image()(PrimaryCCD)) {
            IDMessage(getDeviceName(), "Download complete.");
            ExposureComplete(&PrimaryCCD);
        }
        else {
            DEBUG(INDI::Logger::DBG_ERROR, "Image download failed.");
            PrimaryCCD.setExposureFailed();
        }
    }
    SetTimer(POLLMS);
    return;
}


bool GPhotoCCD::ISNewSwitch(const char* dev, const char* name, ISState* states, char* names[], int n)
{
    try {
        return properties.update(dev, name, states, names, n) || INDI::CCD::ISNewSwitch(dev, name, states, names, n);
    } catch(std::exception &e) {
        log.error() << e.what();
        return false;
    }
}

bool GPhotoCCD::ISNewBLOB(const char* dev, const char* name, int sizes[], int blobsizes[], char* blobs[], char* formats[], char* names[], int n)
{
    try {
        return properties.update(dev, name, sizes, blobsizes, blobs, formats, names, n) || INDI::DefaultDevice::ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
    } catch(std::exception &e) {
        log.error() << e.what();
        return false;
    }
}

bool GPhotoCCD::ISNewNumber(const char* dev, const char* name, double values[], char* names[], int n)
{
    try {
        return properties.update(dev, name, values, names, n) || INDI::CCD::ISNewNumber(dev, name, values, names, n);
    } catch(std::exception &e) {
        log.error() << e.what();
        return false;
    }
}

bool GPhotoCCD::ISNewText(const char* dev, const char* name, char* texts[], char* names[], int n)
{
    try {
        return properties.update(dev, name, const_cast<const char**>(texts), names, n) || INDI::CCD::ISNewText(dev, name, texts, names, n);
    } catch(std::exception &e) {
        log.error() << e.what();
        return false;
    }
}


bool GPhotoCCD::saveConfigItems(FILE* fp)
{
    properties.save_config(fp);
    INDI::CCD::saveConfigItems(fp);
    return true;
}
