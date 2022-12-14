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

#include <ardour/location.h>
#include <ardour/audio_diskstream.h>

#include "ardour_ui.h"
#include "editor.h"
#include "editing.h"
#include "audio_time_axis.h"
#include "region_view.h"
#include "selection.h"

#include "i18n.h"

using namespace ARDOUR;
using namespace PBD;

void
Editor::set_show_waveforms (bool yn)
{
	AudioTimeAxisView* atv;

	if (_show_waveforms != yn) {
		_show_waveforms = yn;
		for (TrackViewList::iterator i = track_views.begin(); i != track_views.end(); ++i) {
			if ((atv = dynamic_cast<AudioTimeAxisView*>(*i)) != 0) {
				atv->set_show_waveforms (yn);
			}
		}
	}
}

void
Editor::set_show_waveforms_recording (bool yn)
{
	AudioTimeAxisView* atv;

	if (_show_waveforms_recording != yn) {
		_show_waveforms_recording = yn;
		for (TrackViewList::iterator i = track_views.begin(); i != track_views.end(); ++i) {
			if ((atv = dynamic_cast<AudioTimeAxisView*>(*i)) != 0) {
				atv->set_show_waveforms_recording (yn);
			}
		}
	}
}

void
Editor::set_show_waveforms_rectified (bool yn)
{
	AudioTimeAxisView* atv;

	if (_show_waveforms_rectified != yn) {
		_show_waveforms_rectified = yn;
		for (TrackViewList::iterator i = track_views.begin(); i != track_views.end(); ++i) {
			if ((atv = dynamic_cast<AudioTimeAxisView*>(*i)) != 0) {
				atv->set_show_waveforms_rectified (yn);
			}
		}
	}
}

gint
Editor::start_updating ()
{
	AudioTimeAxisView* atv;

	//cerr << "Editor::start_updating () called" << endl;//DEBUG
	if (is_mapped() && session) {
		for (TrackViewList::iterator i = track_views.begin(); i != track_views.end(); ++i) {
			if ((atv = dynamic_cast<AudioTimeAxisView*>(*i)) != 0) {
				atv->reset_meter ();
			}
		}
	}

	if (!meters_running) {
		fast_screen_update_connection = ARDOUR_UI::SuperRapidScreenUpdate.connect (mem_fun(*this, &Editor::fast_update_strips));
		meters_running = true;
	}
    return 0;
}

gint
Editor::stop_updating ()
{
	AudioTimeAxisView* atv;
	
	meters_running = false;
	fast_screen_update_connection.disconnect();
	//cerr << "Editor::stop_updating () called" << endl;//DEBUG
	if (is_mapped() && session) {
		for (TrackViewList::iterator i = track_views.begin(); i != track_views.end(); ++i) {
			if ((atv = dynamic_cast<AudioTimeAxisView*>(*i)) != 0) {
				atv->hide_meter ();
			}
		}
	}

    return 0;
}

void
Editor::toggle_meter_updating()
{
	if (Config->get_show_track_meters()) {
		start_updating();
	} else {
		stop_updating ();
	}
	track_canvas_allocate(track_canvas->get_allocation());
}

void
Editor::fast_update_strips ()
{
	AudioTimeAxisView* atv;

	if (is_mapped() && session) {
		for (TrackViewList::iterator i = track_views.begin(); i != track_views.end(); ++i) {
			if ((atv = dynamic_cast<AudioTimeAxisView*>(*i)) != 0) {
				atv->fast_update ();
			}
		}
	}
}

