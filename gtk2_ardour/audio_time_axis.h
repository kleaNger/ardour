/*
    Copyright (C) 2000-2006 Paul Davis 

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

#ifndef __ardour_audio_time_axis_h__
#define __ardour_audio_time_axis_h__

#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/menu.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/radiomenuitem.h>
#include <gtkmm/checkmenuitem.h>

#include <gtkmm2ext/selector.h>
#include <list>

#include <ardour/types.h>

#include "ardour_dialog.h"
#include "route_ui.h"
#include "enums.h"
#include "editing.h"
#include "route_time_axis.h"
#include "canvas.h"

namespace ARDOUR {
	class Session;
	class AudioDiskstream;
	class RouteGroup;
	class Redirect;
	class Insert;
	class Location;
	class AudioPlaylist;
}

class PublicEditor;
class AudioThing;
class AudioStreamView;
class Selection;
class Selectable;
class RegionView;
class AudioRegionView;
class AutomationLine;
class AutomationGainLine;
class AutomationPanLine;
class TimeSelection;
class AutomationTimeAxisView;

class AudioTimeAxisView : public RouteTimeAxisView
{
  public:
 	AudioTimeAxisView (PublicEditor&, ARDOUR::Session&, boost::shared_ptr<ARDOUR::Route>, ArdourCanvas::Canvas& canvas);
 	virtual ~AudioTimeAxisView ();
	
	AudioStreamView* audio_view();

	void set_show_waveforms (bool yn);
	void set_show_waveforms_rectified (bool yn);
	void set_show_waveforms_recording (bool yn);
	void show_all_xfades ();
	void hide_all_xfades ();
	void hide_dependent_views (TimeAxisViewItem&);
	void reveal_dependent_views (TimeAxisViewItem&);
		
	/* Overridden from parent to store display state */
	guint32 show_at (double y, int& nth, Gtk::VBox *parent);
	void hide ();
	
	int set_state (const XMLNode&);
	XMLNode* get_child_xml_node (const string & childname);

	void first_idle ();

	void set_waveform_shape (Editing::WaveformShape);
	void set_waveform_scale (Editing::WaveformScale);

  private:
	friend class AudioStreamView;
	friend class AudioRegionView;
	
	void route_active_changed ();

	void build_automation_action_menu ();
	void append_extra_display_menu_items ();
	
	void toggle_show_waveforms ();
	void toggle_waveforms ();

	void show_all_automation ();
	void show_existing_automation ();
	void hide_all_automation ();

	void add_gain_automation_child ();
	void add_pan_automation_child ();
	void add_parameter_automation_child ();

	void toggle_gain_track ();
	void toggle_pan_track ();

	void gain_hidden ();
	void pan_hidden ();

	void update_pans ();
	void update_control_names ();

	AutomationTimeAxisView* gain_track;
	AutomationTimeAxisView* pan_track;

	// Set from XML so context menu automation buttons can be correctly initialized
	bool show_gain_automation;
	bool show_pan_automation;
	
	Gtk::CheckMenuItem* waveform_item;
	Gtk::RadioMenuItem* traditional_item;
	Gtk::RadioMenuItem* rectified_item;
	Gtk::RadioMenuItem* linearscale_item;
	Gtk::RadioMenuItem* logscale_item;
	Gtk::CheckMenuItem* gain_automation_item;
	Gtk::CheckMenuItem* pan_automation_item;
};

#endif /* __ardour_audio_time_axis_h__ */

