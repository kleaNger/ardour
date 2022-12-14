/*
    Copyright (C) 2000 Paul Davis 

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

#include <cstdlib>
#include <cmath>
#include <string>

#include <pbd/error.h>

#include <ardour/session.h>
#include <ardour/region.h>
#include <gtkmm/treeview.h>

#include "ardour_ui.h"
#include "editor.h"
#include "time_axis_view.h"
#include "region_view.h"
#include "selection.h"

#include "i18n.h"

using namespace ARDOUR;
using namespace PBD;
using namespace sigc;

void
Editor::keyboard_selection_finish (bool add)
{
	if (session && have_pending_keyboard_selection) {

		nframes64_t end;
		bool ignored;

		if (session->transport_rolling()) {
			end = session->audible_frame();
		} else {
			if (!mouse_frame (end, ignored)) {
				return;
			}
		}

		if (add) {
			selection->add (pending_keyboard_selection_start, end);
		} else {
			selection->set (0, pending_keyboard_selection_start, end);
		}

		have_pending_keyboard_selection = false;
	}
}

void
Editor::keyboard_selection_begin ()
{
	if (session) {
		if (session->transport_rolling()) {
			pending_keyboard_selection_start = session->audible_frame();
			have_pending_keyboard_selection = true;
		} else {
			bool ignored;
			nframes64_t where; // XXX fix me

			if (mouse_frame (where, ignored)) {
				pending_keyboard_selection_start = where;
				have_pending_keyboard_selection = true;
			}
				
		}
	}
}

void
Editor::keyboard_paste ()
{
	ensure_entered_track_selected (true);
	paste (1);
}

void
Editor::keyboard_insert_region_list_selection ()
{
	insert_region_list_selection (1);
}

int
Editor::get_prefix (float& val, bool& was_floating)
{
	was_floating = false;
	return 1;
}

