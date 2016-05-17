/*
 * Copyright (C) 2016 Marco Gulino <marco@gulinux.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INDI_GPHOTO_CCD_CAMERA_H
#define INDI_GPHOTO_CCD_CAMERA_H
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <ratio>

namespace INDI {
namespace GPhoto {
class Camera {
public:
  typedef std::chrono::duration<double> Seconds;
  typedef std::shared_ptr<Camera> ptr;
  virtual std::vector<std::string> available_iso() = 0;
  virtual std::string current_iso() = 0;
  virtual bool set_iso(const std::string &iso) = 0;
  
  struct ShootStatus {
    enum Status { Idle, Running, Finished };
    Status status;
    Seconds elapsed;
    Seconds remaining;
  };
  virtual void shoot(Seconds seconds) = 0;
  virtual ShootStatus shoot_status() const = 0;
  
};
}
}

#endif
