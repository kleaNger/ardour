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

#ifndef __ardour_gtk_redirect_automation_time_axis_h__
#define __ardour_gtk_redirect_automation_time_axis_h__

#include <pbd/xml++.h>

#include "canvas.h"
#include "automation_time_axis.h"

namespace ARDOUR {
	class Redirect;
}

class RedirectAutomationTimeAxisView : public AutomationTimeAxisView
{
  public:
	RedirectAutomationTimeAxisView (ARDOUR::Session&,
					boost::shared_ptr<ARDOUR::Route>,
					PublicEditor&,
					TimeAxisView& parent,
					ArdourCanvas::Canvas& canvas,
					std::string name,
					uint32_t port,
					ARDOUR::Redirect& rd,
					std::string state_name);

	~RedirectAutomationTimeAxisView();
	
	void add_automation_event (ArdourCanvas::Item *item, GdkEvent *event, nframes_t, double);

	guint32 show_at (double y, int& nth, Gtk::VBox *parent);
	void hide ();

	
   private:
	ARDOUR::Redirect& redirect;
        uint32_t port;

	XMLNode *xml_node;
	void ensure_xml_node();
	void update_extra_xml_shown (bool editor_shown);

	void set_automation_state (ARDOUR::AutoState);
};

#endif /* __ardour_gtk_redirect_automation_time_axis_h__ */
