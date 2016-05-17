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

#include "simulationcamera.h"
#include <chrono>
#include "logger.h"
using namespace std;
using namespace INDI::GPhoto;

class SimulationCamera::Private {
public:
  Private(INDI::CCD *device, SimulationCamera *q);
  INDI::CCD *device;
  vector<string> avail_iso;
  string current_iso;
  struct Exposure {
    Exposure(Seconds seconds) : seconds{seconds}, started{chrono::steady_clock::now()} {}
    Exposure() : valid{false} {}
    Seconds seconds;
    chrono::steady_clock::time_point started;
    bool valid = true;
    Seconds elapsed() const { return chrono::steady_clock::now() - started; }
    bool finished() const { return elapsed() >= seconds; }
  };
  Exposure exposure;
  Logger log;
private:
  SimulationCamera *q;
};

SimulationCamera::Private::Private(INDI::CCD* device, SimulationCamera* q)
  : device{device}, avail_iso{"100", "200", "400", "800"}, current_iso{"200"}, log{device, "SimulationCamera"}, q{q}
{
}


SimulationCamera::SimulationCamera(INDI::CCD* device) : dptr(device, this)
{
}

SimulationCamera::~SimulationCamera()
{
}

vector< string > SimulationCamera::available_iso()
{
  return d->avail_iso;
}

string SimulationCamera::current_iso()
{
  return d->current_iso;
}

bool SimulationCamera::set_iso(const string& iso)
{
  d->current_iso = iso;
  return true;
}

void SimulationCamera::shoot(Camera::Seconds seconds)
{
  d->log.debug() << "Shooting for " << seconds.count() << ")";
  d->exposure = {seconds};
  // TODO
}

Camera::ShootStatus SimulationCamera::shoot_status() const
{
  if(!d->exposure.valid)
    return {ShootStatus::Idle};
  if(d->exposure.finished())
    return {ShootStatus::Finished, d->exposure.elapsed(), Seconds{0}};
  return {ShootStatus::Running, d->exposure.elapsed(), d->exposure.seconds - d->exposure.elapsed()};
}

INDI::GPhoto::Camera::WriteImage SimulationCamera::write_image() const
{
  return [&](CCDChip &chip){
    d->log.debug() << "Writing image to chip";
    uint8_t * image = chip.getFrameBuffer();

    // Get width and height
    int width = chip.getSubW() / chip.getBinX() * chip.getBPP()/8;
    int height = chip.getSubH() / chip.getBinY();
    d->log.debug() << "w=" << width << ", h=" << height;

    // Fill buffer with random pattern
    for (int i=0; i < height ; i++) {
	d->log.debug() << "Row: " << i;
        for (int j=0; j < width; j++)
            image[i*width+j] = rand() % 255;
    }
    d->log.debug() << "Finished generating random image";
    return true;
  };
}

