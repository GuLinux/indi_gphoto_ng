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

#ifndef REALCAMERA_H
#define REALCAMERA_H

#include "camera.h"
#include "c++/dptr.h"
#include <indiccd.h>
namespace INDI {
namespace GPhoto {
class RealCamera : public INDI::GPhoto::Camera
{
public:
    RealCamera(INDI::CCD *device);
    ~RealCamera();
    virtual std::vector< std::string > available_iso();
    virtual std::string current_iso();
    bool set_iso(const std::__cxx11::string& iso);
private:
  D_PTR;
};
}
}

#endif // REALCAMERA_H
