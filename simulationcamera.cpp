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
using namespace std;
using namespace INDI::GPhoto;

class SimulationCamera::Private {
public:
  Private(SimulationCamera *q);
  vector<string> avail_iso;
  string current_iso;
private:
  SimulationCamera *q;
};

SimulationCamera::Private::Private(SimulationCamera* q) : avail_iso{"100", "200", "400", "800"}, current_iso{"200"}, q{q}
{
}


SimulationCamera::SimulationCamera() : dptr(this)
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
