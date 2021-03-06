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
#include "c++/containers_streams.h"
using namespace std;
using namespace GuLinux;
using namespace INDI::GPhoto;
using namespace GPhotoCPP;
using namespace INDI::Properties;
class RealCamera::Private {
public:
    enum ImageType { RAW, JPEG };
    Private(INDI::CCD *device, RealCamera *q);
    INDI::CCD *device;
    INDI::Utils::Logger log;
    map<ImageType, GPhotoCPP::ReadImage::ptr> image_parsers;
    shared_ptr< GPhotoCPP::Logger > gphoto_logger;
    shared_ptr< GPhotoCPP::Driver > driver;
    GPhotoCPP::CameraPtr camera;
    GPhotoCPP::Camera::ShotPtr current_shoot;
    Seconds mirror_lock = Seconds{0};
    list<string> used_widget_names;
    template<typename T> shared_ptr<T> widget_value(const string &name);
private:
    RealCamera *q;
};

RealCamera::Private::Private(INDI::CCD* device, RealCamera* q)
    : device {device},
      log {device, "GPhotoCamera"},
      image_parsers {{RAW, make_shared<GPhotoCPP::ReadRawImage>()}, {JPEG, make_shared<GPhotoCPP::ReadJPEGImage>()}},
      q{q}
{
    gphoto_logger = make_shared<GPhotoCPP::Logger>([=](const string &m, GPhotoCPP::Logger::Level l) {
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
    used_widget_names = make_stream(list<GPhotoCPP::WidgetPtr>{camera->settings().iso_widget(), camera->settings().format_widget()})
	.filter([](const GPhotoCPP::WidgetPtr &w) -> bool { return w.operator bool(); })
	.transform<list<string>>([](const GPhotoCPP::WidgetPtr &w){ return w->name(); })
	.get();
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

vector< string > RealCamera::available_formats()
{
  return d->camera->settings().format_choices();
}

string RealCamera::current_format()
{
  return d->camera->settings().format();
}

bool RealCamera::set_format(const string& format)
{
    d->camera->settings().set_format(format);
    d->camera->save_settings();
    return current_format() == format;
}




bool RealCamera::shoot(INDI::GPhoto::Camera::Seconds seconds)
{
    if(current_format().find('+') != string::npos) {
      d->log.error() << "Error! current format " << current_format() << 
	" is not supported. Please select only a single format (RAW or JPEG), not composite formats (RAW+JPEG)";
      return false;
    }
    bool mirror_lock_enabled = d->mirror_lock > Seconds{0};
    d->log.debug() << "mirrorlock secs: " << d->mirror_lock.count() << ", enabled: " << mirror_lock_enabled;
    d->current_shoot = d->camera->control().shoot(seconds, mirror_lock_enabled, d->mirror_lock);
    return d->current_shoot.operator bool();
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
    return [&](CCDChip &chip) {
        d->current_shoot->camera_file().wait();
        d->log.session() << "Exposure complete, downloading image...";
        GPhotoCPP::CameraFilePtr  file = d->current_shoot->camera_file().get();
        d->current_shoot.reset();
        string extension = make_stream(file->file().substr(file->file().rfind(".")+1)).transform<string>(::tolower);
        auto image_parser = (extension == "jpg" || extension == "jpeg") ? d->image_parsers[Private::JPEG] : d->image_parsers[Private::RAW];
        d->log.debug() << "Image filename" << file->file() << ", extension: " << extension;
        vector<uint8_t> original_data = file->data();
        GPhotoCPP::ReadImage::Image image;
        try {
            image = image_parser->read(original_data, file->file());
        } catch(std::exception &e) {
            d->log.error() << "Exposure failed to parse image: " << e.what();
            return false;
        }
        d->log.debug() << "Copying image: w=" << image.w << ", h=" << image.h << ", bpp=" << image.bpp << ", channels=" << image.channels.size();
        chip.setFrame(0, 0, image.w, image.h);
        chip.setResolution(image.w, image.h);
        chip.setNAxis(image.channels.size() == 3 ? 3 : 2);
        chip.setBPP(image.bpp);

        typedef std::pair<GPhotoCPP::ReadImage::Image::Channel, GPhotoCPP::ReadImage::Image::Pixels> channel;
        chip.setFrameBufferSize(make_stream(image.channels).transform<list<size_t>>([](const channel &c) {
            return c.second.size();
        }).accumulate(), true);
        size_t data_begin = 0;
        for(auto c: image.channels) {
            std::move(c.second.begin(), c.second.end(), chip.getFrameBuffer() + data_begin);
            data_begin += c.second.size();
        }
        chip.setImageExtension("fits");
        return true;
    };
}

template<typename T> shared_ptr<T> RealCamera::Private::widget_value(const string& name)
{
  return camera->widgets_settings()->child_by_name(name)->get<T>();
}


void RealCamera::setup_properties(::Properties< std::string >& properties)
{
    typedef GPhotoCPP::Widget Widget;
    typedef const GPhotoCPP::WidgetPtr& WidgetPtr;
    auto make_identity = [this](WidgetPtr w) {
        return Identity {d->device->getDeviceName(), w->name(), w->label(), w->parent()->label(), w->access() == GPhotoCPP::Widget::ReadOnly ? IP_RO : IP_RW};
    };
    map<GPhotoCPP::Widget::Type, function<void(WidgetPtr)>> supported_types {
        {   Widget::String, [&](WidgetPtr w) {
		auto wv = [=] { return d->widget_value<Widget::StringValue>(w->name()); };
                auto widget_value = wv();
                properties.add_text(w->name(), d->device, make_identity(w), [=](const vector<Text::UpdateArgs> &u) {
                    string value = get<0>(u[0]);
                    wv()->set(value);
                    d->camera->save_settings();
                    return wv()->get() == value;
                })
                .add(w->name(), w->label(), widget_value->get().c_str());
            }
        },
        {   Widget::Range, [&](WidgetPtr w) {
		auto wv = [=] { return d->widget_value<Widget::RangeValue>(w->name()); };
		auto widget_value = wv();
                properties.add_number(w->name(), d->device, make_identity(w), [=](const vector<Number::UpdateArgs> &u) {
                    auto value= get<0>(u[0]);
                    wv()->set(value);
                    d->camera->save_settings();
                    return wv()->get() == value;
                })
                .add(w->name(), w->label(), widget_value->range().min, widget_value->range().max, widget_value->range().increment, widget_value->get());
            }
        },
        {   Widget::Toggle, [&](WidgetPtr w) {
		auto wv = [=] { return d->widget_value<Widget::ToggleValue>(w->name()); };
		bool is_on = wv()->get();
                properties.add_switch(w->name(), d->device, make_identity(w), ISR_1OFMANY, [=](const vector<Switch::UpdateArgs> &u) {
                    bool is_on = get<1>(u[0]) == "on";
		    wv()->set(is_on);
                    d->camera->save_settings();
                    return wv()->get() == is_on;
                })
                .add("on", "On", is_on ? ISS_ON : ISS_OFF)
                .add("off", "Off", is_on ? ISS_OFF : ISS_ON);
            }
        },
        {   Widget::Menu, [&](WidgetPtr w) {
		auto wv = [=] { return d->widget_value<Widget::MenuValue>(w->name()); };
                auto widget_value = wv();
                auto &sw = properties.add_switch(w->name(), d->device, make_identity(w), ISR_1OFMANY, [=](const vector<Switch::UpdateArgs> &u) {
                    auto current_text = get<1>(*make_stream(u).first(Switch::On));
                    wv()->set(current_text);
                    d->camera->save_settings();
                    return wv()->get() == current_text;
                });
		auto current_choice = widget_value->get();
		for(auto choice: widget_value->choices()) {
		  sw.add(choice, choice, choice == current_choice ? ISS_ON : ISS_OFF);
		}
            }
        },
        { Widget::Button, {} },
        { Widget::Date, {} },
        { Widget::Window, {} },
        { Widget::Section, {} },
    };
    auto format_widget = d->camera->settings().format_widget();
    auto iso_widget = d->camera->settings().iso_widget();
    auto widgets = make_stream(d->camera->widgets_settings()->all_children())
    .filter([&](WidgetPtr w) {
        return supported_types[w->type()] && ! make_stream(d->used_widget_names).contains(w->name());
    })
    .for_each([&](WidgetPtr w) {
        supported_types[w->type()](w);
    });
    if(d->camera->settings().needs_serial_port()) {
      properties.add_text("serial_port", d->device, {d->device->getDeviceName(), "trigger", "Trigger", "Main Control", IP_RW}, [=](const vector<Text::UpdateArgs> &u) {
	d->camera->settings().set_serial_port(get<0>(u[0]));
	return true;
      }).add("serial_port_value", "Serial Port", "");
    }
    properties.add_number("mirror_lock", d->device, {d->device->getDeviceName(), "mirror_lock", "Mirror Lock", "Main Control", IP_RW}, [=](const vector<Number::UpdateArgs> &u) {
      d->log.debug() << "Setting mirror lock to " << get<0>(u[0]);
      d->mirror_lock = Seconds{get<0>(u[0])};
      return true;
    }).add("mirrorlock_sec", "seconds", 0, 10, 1, d->mirror_lock.count(), "%1.0f");
}
