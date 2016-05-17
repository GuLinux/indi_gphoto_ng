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

#ifndef SIMULATIONCAMERA_H
#define SIMULATIONCAMERA_H

#include "camera.h"
#include "c++/dptr.h"
namespace INDI {
namespace GPhoto {
class SimulationCamera : public INDI::GPhoto::Camera
{
public:
    SimulationCamera();
    ~SimulationCamera();
    virtual std::vector< std::string > available_iso();
    virtual std::string current_iso();
    virtual bool set_iso(const std::string &iso);
    
    virtual void shoot(Seconds seconds);
    virtual ShootStatus shoot_status() const;
private:
  D_PTR;
};
}
}
#endif // SIMULATIONCAMERA_H
