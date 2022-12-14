/*
    Copyright (C) 2000-2007 Paul Davis 

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

#include <iostream>

#include "panner.h"

using namespace std;

static const int triangle_size = 5;

static void
null_label_callback (char* buf, unsigned int bufsize)
{
	/* no label */

	buf[0] = '\0';
}


PannerBar::PannerBar (Gtk::Adjustment& adj, PBD::Controllable& c)
	: BarController (adj, c, sigc::ptr_fun (null_label_callback))
{
	set_style (BarController::Line);
}

PannerBar::~PannerBar ()
{
}

bool
PannerBar::expose (GdkEventExpose* ev)
{
	Glib::RefPtr<Gdk::Window> win (darea.get_window());
	Glib::RefPtr<Gdk::GC> gc (get_style()->get_base_gc (get_state()));

	BarController::expose (ev);

	/* now draw triangles for left, right and center */

	GdkPoint points[3];

	// left
	
	points[0].x = 0;
	points[0].y = 0;

	points[1].x = triangle_size;
	points[1].y = 0;
	
	points[2].x = 0;
	points[2].y = triangle_size;

	gdk_draw_polygon (win->gobj(), gc->gobj(), true, points, 3);

	// center

	points[0].x = (darea.get_width()/2 - triangle_size);
	points[0].y = 0;

	points[1].x = (darea.get_width()/2 + triangle_size);
	points[1].y = 0;
	
	points[2].x = darea.get_width()/2;
	points[2].y = triangle_size - 1;

	gdk_draw_polygon (win->gobj(), gc->gobj(), true, points, 3); 

	// right

	points[0].x = (darea.get_width() - triangle_size);
	points[0].y = 0;

	points[1].x = darea.get_width();
	points[1].y = 0;
	
	points[2].x = darea.get_width();
	points[2].y = triangle_size;

	gdk_draw_polygon (win->gobj(), gc->gobj(), true, points, 3);

	return true;
}

bool
PannerBar::button_press (GdkEventButton* ev)
{
	if (ev->button == 1 && ev->type == GDK_BUTTON_PRESS && ev->y < 10) {
		if (ev->x < triangle_size) {
			adjustment.set_value (adjustment.get_lower());
		} else if (ev->x > (darea.get_width() - triangle_size)) {
			adjustment.set_value (adjustment.get_upper());
		} else if (ev->x > (darea.get_width()/2 - triangle_size) &&
			   ev->x < (darea.get_width()/2 + triangle_size)) {
			adjustment.set_value (adjustment.get_lower() + ((adjustment.get_upper() - adjustment.get_lower()) / 2.0));
		}
	}

	return BarController::button_press (ev);
}

bool
PannerBar::button_release (GdkEventButton* ev)
{
	return BarController::button_release (ev);
}

