/*
    Copyright (C) 1998-2006 Paul Davis 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef __gtkmm2ext_slider_controller_h__
#define __gtkmm2ext_slider_controller_h__

#include <gtkmm.h>
#include <gtkmm2ext/popup.h>
#include <gtkmm2ext/pixfader.h>
#include <gtkmm2ext/binding_proxy.h>

namespace Gtkmm2ext {
	class Pix;
}

namespace PBD {
	class Controllable;
}

namespace Gtkmm2ext {

class SliderController : public Gtkmm2ext::PixFader
{
  public:
	SliderController (Glib::RefPtr<Gdk::Pixbuf> image,
			  Gtk::Adjustment* adj, int orientation,
			  PBD::Controllable*,
			  bool with_numeric = true);

        virtual ~SliderController () {}

	void set_value (float);

	Gtk::SpinButton& get_spin_button () { return spin; }
	
	bool on_button_press_event (GdkEventButton *ev);

	void set_controllable (PBD::Controllable* c) { binding_proxy.set_controllable (c); }

  protected:
	BindingProxy binding_proxy;
	Glib::RefPtr<Gdk::Pixbuf> slider;
	Glib::RefPtr<Gdk::Pixbuf> rail;
	Gtk::SpinButton     spin;
	Gtk::Frame          spin_frame;
	Gtk::HBox           spin_hbox;
};

class VSliderController : public SliderController
{
  public:
	VSliderController (Glib::RefPtr<Gdk::Pixbuf> image,
			   Gtk::Adjustment *adj,
			   PBD::Controllable*,
			   bool with_numeric = true);
};

class HSliderController : public SliderController
{
  public:
	HSliderController (Glib::RefPtr<Gdk::Pixbuf> image,
			   Gtk::Adjustment *adj,
			   PBD::Controllable*,
			   bool with_numeric = true);
};


}; /* namespace */

#endif // __gtkmm2ext_slider_controller_h__		
