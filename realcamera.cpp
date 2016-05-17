/*
 * Driver type: GPhoto Camera INDI Driver
 *
 * Copyright (C) 2016 Marco Gulino (marco AT gulinux.net)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "realcamera.h"
#include "logger.h"
#include "GPhoto++.h"

using namespace std;
using namespace INDI::GPhoto;
class RealCamera::Private {
public:
    Private(INDI::CCD *device, RealCamera *q);
    INDI::CCD *device;
    Logger log;
    shared_ptr< GPhotoCPP::Logger > gphoto_logger;
    shared_ptr< GPhotoCPP::Driver > driver;
    GPhotoCPP::CameraPtr camera;
    GPhotoCPP::Camera::ShotPtr current_shoot;
private:
    RealCamera *q;
};

RealCamera::Private::Private(INDI::CCD* device, RealCamera* q) : device{device}, log {device, "GPhotoCamera"}, q{q}
{
    gphoto_logger = make_shared<GPhotoCPP::Logger>([&](const string &m, GPhotoCPP::Logger::Level l) {
        static map<GPhotoCPP::Logger::Level, INDI::Logger::VerbosityLevel> levels {
            {GPhotoCPP::Logger::ERROR, INDI::Logger::DBG_ERROR },
            {GPhotoCPP::Logger::WARNING, INDI::Logger::DBG_WARNING },
            {GPhotoCPP::Logger::INFO, INDI::Logger::DBG_SESSION },
            {GPhotoCPP::Logger::DEBUG, INDI::Logger::DBG_DEBUG },
            {GPhotoCPP::Logger::TRACE, INDI::Logger::DBG_EXTRA_1 },
        };
        DEBUGDEVICE(device->getDeviceName(), levels[l], m.c_str());
    });
    driver = make_shared<GPhotoCPP::Driver>(gphoto_logger);
    camera =  driver->autodetect();
    if(! camera)
      throw std::runtime_error("Unable to find camera");
}


RealCamera::RealCamera(INDI::CCD* device) : dptr(device, this)
{
}

RealCamera::~RealCamera()
{
}

vector< string > RealCamera::available_iso()
{
  return d->camera->settings().iso_choices();
}

string RealCamera::current_iso()
{
  return d->camera->settings().iso();
}

bool RealCamera::set_iso(const string& iso)
{
  d->camera->settings().set_iso(iso);
  d->camera->save_settings();
  return current_iso() == iso;
}

void RealCamera::shoot(INDI::GPhoto::Camera::Seconds seconds)
{
  d->current_shoot = d->camera->control().shoot(seconds);
}

INDI::GPhoto::Camera::ShootStatus RealCamera::shoot_status() const
{
  if(! d->current_shoot )
    return {Camera::ShootStatus::Idle};
  if( d->current_shoot->elapsed() >= d->current_shoot->duration() )
    return {Camera::ShootStatus::Finished, d->current_shoot->elapsed(), Seconds{0} };
  return {Camera::ShootStatus::Finished, d->current_shoot->elapsed(), d->current_shoot->duration() - d->current_shoot->elapsed() };
}

INDI::GPhoto::Camera::WriteImage RealCamera::write_image() const
{
  return [&](CCDChip &chip){
    // TODO
    d->current_shoot.reset();
    return false;
  };
}


void RealCamera::setup_properties(INDI::Properties::Properties< std::string >& properties)
{
}
