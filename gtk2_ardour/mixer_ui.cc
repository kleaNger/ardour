/*
    Copyright (C) 2000-2004 Paul Davis 

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

#include <algorithm>
#include <sigc++/bind.h>

#include <gtkmm/accelmap.h>

#include <pbd/convert.h>
#include <pbd/stacktrace.h>
#include <glibmm/thread.h>

#include <gtkmm2ext/gtk_ui.h>
#include <gtkmm2ext/utils.h>
#include <gtkmm2ext/stop_signal.h>
#include <gtkmm2ext/window_title.h>

#include <ardour/session.h>
#include <ardour/audio_track.h>
#include <ardour/session_route.h>
#include <ardour/audio_diskstream.h>
#include <ardour/plugin_manager.h>

#include "keyboard.h"
#include "mixer_ui.h"
#include "mixer_strip.h"
#include "plugin_selector.h"
#include "ardour_ui.h"
#include "prompter.h"
#include "utils.h"
#include "actions.h"
#include "gui_thread.h"

#include "i18n.h"

using namespace ARDOUR;
using namespace PBD;
using namespace Gtk;
using namespace Glib;
using namespace Gtkmm2ext;
using namespace sigc;
using namespace std;

using PBD::atoi;

static void
pane_size_watcher (Paned* pane)
{
	/* if the handle of a pane vanishes into (at least) the tabs of a notebook,
	   it is no longer accessible. so stop that. this doesn't happen on X11,
	   just the quartz backend.
	   
	   ugh.
	*/

	gint pos = pane->get_position ();

	if (pos < 22) {
		pane->set_position (22);
	}
}



Mixer_UI::Mixer_UI ()
	: Window (Gtk::WINDOW_TOPLEVEL)
{
	session = 0;
	_strip_width = Config->get_default_narrow_ms() ? Narrow : Wide;
	track_menu = 0;
	mix_group_context_menu = 0;
	no_track_list_redisplay = false;
	in_group_row_change = false;
	_visible = false;
	strip_redisplay_does_not_reset_order_keys = false;
	strip_redisplay_does_not_sync_order_keys = false;

	Route::SyncOrderKeys.connect (mem_fun (*this, &Mixer_UI::sync_order_keys));

 	scroller_base.add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK);
 	scroller_base.set_name ("MixerWindow");
 	scroller_base.signal_button_release_event().connect (mem_fun(*this, &Mixer_UI::strip_scroller_button_release));
	// add as last item of strip packer
	strip_packer.pack_end (scroller_base, true, true);

	scroller.add (strip_packer);
	scroller.set_policy (Gtk::POLICY_ALWAYS, Gtk::POLICY_AUTOMATIC);

	track_model = ListStore::create (track_columns);
	track_display.set_model (track_model);
	/* LT change column order based on value of TRACK_DISPLAY_... enum. */
	if (TRACK_DISPLAY_SHOW_COLUMN < TRACK_DISPLAY_STRIPS_COLUMN) {
	  track_display.append_column (_("Show"), track_columns.visible);
	  track_display.append_column (_("Strips"), track_columns.text);
	} else {
	  track_display.append_column (_("Strips"), track_columns.text);
	  track_display.append_column (_("Show"), track_columns.visible);
	}
	track_display.get_column (TRACK_DISPLAY_STRIPS_COLUMN)->set_data (X_("colnum"), GUINT_TO_POINTER(TRACK_DISPLAY_STRIPS_COLUMN));
	track_display.get_column (TRACK_DISPLAY_SHOW_COLUMN)->set_data (X_("colnum"), GUINT_TO_POINTER(TRACK_DISPLAY_SHOW_COLUMN));
	track_display.get_column (TRACK_DISPLAY_STRIPS_COLUMN)->set_expand(true);
	track_display.get_column (TRACK_DISPLAY_SHOW_COLUMN)->set_expand(false);
	track_display.set_name (X_("MixerTrackDisplayList"));
	track_display.get_selection()->set_mode (Gtk::SELECTION_NONE);
	track_display.set_reorderable (true);
	track_display.set_headers_visible (true);

	track_model->signal_row_deleted().connect (mem_fun (*this, &Mixer_UI::track_list_delete));
	track_model->signal_row_changed().connect (mem_fun (*this, &Mixer_UI::track_list_change));
	track_model->signal_rows_reordered().connect (mem_fun (*this, &Mixer_UI::track_list_reorder));

	CellRendererToggle* track_list_visible_cell = dynamic_cast<CellRendererToggle*>(track_display.get_column_cell_renderer (TRACK_DISPLAY_SHOW_COLUMN));
	track_list_visible_cell->property_activatable() = true;
	track_list_visible_cell->property_radio() = false;

	track_display.signal_button_press_event().connect (mem_fun (*this, &Mixer_UI::track_display_button_press), false);

	track_display_scroller.add (track_display);
	track_display_scroller.set_policy (Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

	group_model = ListStore::create (group_columns);
	group_display.set_model (group_model);
	/* LT change column order based on value of GROUP_DISPLAY_... enum. */
	/* LT Code only two possibilities : Group, Active, Show or Show, Active, Group. */
	if (GROUP_DISPLAY_SHOW_COLUMN < GROUP_DISPLAY_ACTIVE_COLUMN) {
	  group_display.append_column (_("Show"), group_columns.visible);
	  group_display.append_column (_("Active"), group_columns.active);
	  group_display.append_column (_("Group"), group_columns.text);
	} else {
	  group_display.append_column (_("Group"), group_columns.text);
	  group_display.append_column (_("Active"), group_columns.active);
	  group_display.append_column (_("Show"), group_columns.visible);
	}
	group_display.get_column (GROUP_DISPLAY_GROUP_COLUMN)->set_data (X_("colnum"), GUINT_TO_POINTER(GROUP_DISPLAY_GROUP_COLUMN));
	group_display.get_column (GROUP_DISPLAY_ACTIVE_COLUMN)->set_data (X_("colnum"), GUINT_TO_POINTER(GROUP_DISPLAY_ACTIVE_COLUMN));
	group_display.get_column (GROUP_DISPLAY_SHOW_COLUMN)->set_data (X_("colnum"), GUINT_TO_POINTER(GROUP_DISPLAY_SHOW_COLUMN));
	group_display.get_column (GROUP_DISPLAY_GROUP_COLUMN)->set_expand(true);
	group_display.get_column (GROUP_DISPLAY_ACTIVE_COLUMN)->set_expand(false);
	group_display.get_column (GROUP_DISPLAY_SHOW_COLUMN)->set_expand(false);
	group_display.set_name ("MixerGroupList");
	group_display.get_selection()->set_mode (Gtk::SELECTION_SINGLE);
	group_display.set_reorderable (true);
	group_display.set_headers_visible (true);
	group_display.set_rules_hint (true);

	/* name is directly editable */

	CellRendererText* name_cell = dynamic_cast<CellRendererText*>(group_display.get_column_cell_renderer (GROUP_DISPLAY_GROUP_COLUMN));
	name_cell->property_editable() = true;
	name_cell->signal_edited().connect (mem_fun (*this, &Mixer_UI::mix_group_name_edit));

	/* use checkbox for the active column */

	CellRendererToggle* active_cell = dynamic_cast<CellRendererToggle*>(group_display.get_column_cell_renderer (GROUP_DISPLAY_ACTIVE_COLUMN));
	active_cell->property_activatable() = true;
	active_cell->property_radio() = false;

	/* use checkbox for the visible column */

	active_cell = dynamic_cast<CellRendererToggle*>(group_display.get_column_cell_renderer (GROUP_DISPLAY_SHOW_COLUMN));
	active_cell->property_activatable() = true;
	active_cell->property_radio() = false;

	group_model->signal_row_changed().connect (mem_fun (*this, &Mixer_UI::mix_group_row_change));

	group_display.signal_button_press_event().connect (mem_fun (*this, &Mixer_UI::group_display_button_press), false);

	group_display_scroller.add (group_display);
	group_display_scroller.set_policy (Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

	HBox* mix_group_display_button_box = manage (new HBox());

	Button* mix_group_add_button = manage (new Button ());
	Button* mix_group_remove_button = manage (new Button ());

	Widget* w;

	w = manage (new Image (Stock::ADD, ICON_SIZE_BUTTON));
	w->show();
	mix_group_add_button->add (*w);

	w = manage (new Image (Stock::REMOVE, ICON_SIZE_BUTTON));
	w->show();
	mix_group_remove_button->add (*w);

	mix_group_display_button_box->set_homogeneous (true);

	mix_group_add_button->signal_clicked().connect (mem_fun (*this, &Mixer_UI::new_mix_group));
	mix_group_remove_button->signal_clicked().connect (mem_fun (*this, &Mixer_UI::remove_selected_mix_group));

	mix_group_display_button_box->add (*mix_group_remove_button);
	mix_group_display_button_box->add (*mix_group_add_button);

	group_display_vbox.pack_start (group_display_scroller, true, true);
	group_display_vbox.pack_start (*mix_group_display_button_box, false, false);

	track_display_frame.set_name("BaseFrame");
	track_display_frame.set_shadow_type (Gtk::SHADOW_IN);
	track_display_frame.add(track_display_scroller);

	group_display_frame.set_name ("BaseFrame");
	group_display_frame.set_shadow_type (Gtk::SHADOW_IN);
	group_display_frame.add (group_display_vbox);

	rhs_pane1.pack1 (track_display_frame);
	rhs_pane1.pack2 (group_display_frame);

	list_vpacker.pack_start (rhs_pane1, true, true);

	global_hpacker.pack_start (scroller, true, true);
#ifdef GTKOSX
	/* current gtk-quartz has dirty updates on borders like this one */
	global_hpacker.pack_start (out_packer, false, false, 0);
#else
	global_hpacker.pack_start (out_packer, false, false, 12);
#endif
	list_hpane.add1(list_vpacker);
	list_hpane.add2(global_hpacker);

	rhs_pane1.signal_size_allocate().connect (bind (mem_fun(*this, &Mixer_UI::pane_allocation_handler), 
							static_cast<Gtk::Paned*> (&rhs_pane1)));
	list_hpane.signal_size_allocate().connect (bind (mem_fun(*this, &Mixer_UI::pane_allocation_handler), 
							 static_cast<Gtk::Paned*> (&list_hpane)));

	Glib::PropertyProxy<int> proxy = list_hpane.property_position();
	proxy.signal_changed().connect (bind (sigc::ptr_fun (pane_size_watcher), static_cast<Paned*> (&list_hpane)));

	
	global_vpacker.pack_start (list_hpane, true, true);

	add (global_vpacker);
	set_name ("MixerWindow");
	
	WindowTitle title(Glib::get_application_name());
	title += _("Mixer");
	set_title (title.get_string());

	set_wmclass (X_("ardour_mixer"), PROGRAM_NAME);

	add_accel_group (ActionManager::ui_manager->get_accel_group());

	signal_delete_event().connect (mem_fun (*this, &Mixer_UI::hide_window));
	add_events (Gdk::KEY_PRESS_MASK|Gdk::KEY_RELEASE_MASK);

	_plugin_selector = new PluginSelector (PluginManager::the_manager());

	signal_configure_event().connect (mem_fun (*ARDOUR_UI::instance(), &ARDOUR_UI::configure_handler));

	_selection.RoutesChanged.connect (mem_fun(*this, &Mixer_UI::follow_strip_selection));

	auto_rebinding = FALSE;
}

Mixer_UI::~Mixer_UI ()
{
}

void
Mixer_UI::ensure_float (Window& win)
{
	win.set_transient_for (*this);
}

void
Mixer_UI::show_window ()
{
	show_all ();
	if (!_visible) {
		set_window_pos_and_size ();

		/* now reset each strips width so the right widgets are shown */
		MixerStrip* ms;
		
		TreeModel::Children rows = track_model->children();
		TreeModel::Children::iterator ri;
		
		for (ri = rows.begin(); ri != rows.end(); ++ri) {
			ms = (*ri)[track_columns.strip];
			ms->set_width (ms->get_width(), ms->width_owner());
		}
	}
	_visible = true;
}

bool
Mixer_UI::hide_window (GdkEventAny *ev)
{
	get_window_pos_and_size ();

	_visible = false;
	return just_hide_it(ev, static_cast<Gtk::Window *>(this));
}


void
Mixer_UI::add_strip (Session::RouteList& routes)
{
	ENSURE_GUI_THREAD(bind (mem_fun(*this, &Mixer_UI::add_strip), routes));
	
	MixerStrip* strip;

	no_track_list_redisplay = true;
	strip_redisplay_does_not_sync_order_keys = true;

	for (Session::RouteList::iterator x = routes.begin(); x != routes.end(); ++x) {
		boost::shared_ptr<Route> route = (*x);

		if (route->hidden()) {
			return;
		}
		
		strip = new MixerStrip (*this, *session, route);
		strips.push_back (strip);

		Config->get_default_narrow_ms() ? _strip_width = Narrow : _strip_width = Wide;

		if (strip->width_owner() != strip) {
			strip->set_width (_strip_width, this);
		}

		show_strip (strip);
		
		TreeModel::Row row = *(track_model->append());
		row[track_columns.text] = route->name();
		row[track_columns.visible] = strip->marked_for_display();
		row[track_columns.route] = route;
		row[track_columns.strip] = strip;
		
		if (route->order_key (N_("signal")) == -1) {
			route->set_order_key (N_("signal"), track_model->children().size()-1);
		}
		
		route->name_changed.connect (bind (mem_fun(*this, &Mixer_UI::strip_name_changed), strip));

		strip->GoingAway.connect (bind (mem_fun(*this, &Mixer_UI::remove_strip), strip));
#ifdef GTKOSX
		strip->WidthChanged.connect (mem_fun(*this, &Mixer_UI::queue_draw_all_strips));
#endif	
		strip->signal_button_release_event().connect (bind (mem_fun(*this, &Mixer_UI::strip_button_release_event), strip));
	}

	no_track_list_redisplay = false;

	redisplay_track_list ();
	
	strip_redisplay_does_not_sync_order_keys = false;
}

void
Mixer_UI::remove_strip (MixerStrip* strip)
{
	ENSURE_GUI_THREAD(bind (mem_fun(*this, &Mixer_UI::remove_strip), strip));
	
	TreeModel::Children rows = track_model->children();
	TreeModel::Children::iterator ri;
	list<MixerStrip *>::iterator i;

	if ((i = find (strips.begin(), strips.end(), strip)) != strips.end()) {
		strips.erase (i);
	}

	strip_redisplay_does_not_sync_order_keys = true;

	for (ri = rows.begin(); ri != rows.end(); ++ri) {
		if ((*ri)[track_columns.strip] == strip) {
			track_model->erase (ri);
			break;
		}
	}

	strip_redisplay_does_not_sync_order_keys = false;
}

const char*
Mixer_UI::get_order_key() 
{
	return X_("signal");

	if (Config->get_sync_all_route_ordering()) {
		return X_("editor");
	} else {
		return X_("signal");
	}

}


void
Mixer_UI::sync_order_keys (const char *src)
{
	vector<int> neworder;
	TreeModel::Children rows = track_model->children();
	TreeModel::Children::iterator ri;

	if ((strcmp (src, get_order_key()) == 0) || !session || (session->state_of_the_state() & Session::Loading) || rows.empty()) {
		return;
	}

	for (ri = rows.begin(); ri != rows.end(); ++ri) {
		neworder.push_back (0);
	}

	bool changed = false;
	int order;

	for (order = 0, ri = rows.begin(); ri != rows.end(); ++ri, ++order) {
		boost::shared_ptr<Route> route = (*ri)[track_columns.route];
		int old_key = order;
		int new_key = route->order_key (get_order_key());

		neworder[new_key] = old_key;

		if (new_key != old_key) {
			changed = true;
		}
	}

	if (changed) {
		strip_redisplay_does_not_reset_order_keys = true;
		track_model->reorder (neworder);
		strip_redisplay_does_not_reset_order_keys = false;
	}
}


void
Mixer_UI::follow_strip_selection ()
{
	for (list<MixerStrip *>::iterator i = strips.begin(); i != strips.end(); ++i) {
		(*i)->set_selected (_selection.selected ((*i)->route()));
	}
}

bool
Mixer_UI::strip_button_release_event (GdkEventButton *ev, MixerStrip *strip)
{
	if (ev->button == 1) {

		/* this allows the user to click on the strip to terminate comment
		   editing. XXX it needs improving so that we don't select the strip
		   at the same time.
		*/
		
		if (_selection.selected (strip->route())) {
			_selection.remove (strip->route());
		} else {
			if (Keyboard::modifier_state_equals (ev->state, Keyboard::TertiaryModifier)) {
				_selection.add (strip->route());
			} else {
				_selection.set (strip->route());
			}
		}
	}

	return true;
}

void
Mixer_UI::connect_to_session (Session* sess)
{
	session = sess;

	XMLNode* node = ARDOUR_UI::instance()->mixer_settings();
	set_state (*node);

	WindowTitle title(session->name());
	title += _("Mixer");
	title += Glib::get_application_name();

	set_title (title.get_string());

	initial_track_display ();

	session->GoingAway.connect (mem_fun(*this, &Mixer_UI::disconnect_from_session));
	session->RouteAdded.connect (mem_fun(*this, &Mixer_UI::add_strip));
	session->mix_group_added.connect (mem_fun(*this, &Mixer_UI::add_mix_group));
	session->mix_group_removed.connect (mem_fun(*this, &Mixer_UI::mix_groups_changed));

	mix_groups_changed ();
	
	_plugin_selector->set_session (session);

	if (_visible) {
	       show_window();
	}

	start_updating ();
}

void
Mixer_UI::disconnect_from_session ()
{
	ENSURE_GUI_THREAD(mem_fun(*this, &Mixer_UI::disconnect_from_session));
	
	group_model->clear ();
	_selection.clear ();

	WindowTitle title(Glib::get_application_name());
	title += _("Mixer");
	set_title (title.get_string());
	
	stop_updating ();
}

void
Mixer_UI::show_strip (MixerStrip* ms)
{
	TreeModel::Children rows = track_model->children();
	TreeModel::Children::iterator i;
	
	for (i = rows.begin(); i != rows.end(); ++i) {
	
		MixerStrip* strip = (*i)[track_columns.strip];
		if (strip == ms) {
			(*i)[track_columns.visible] = true;
			break;
		}
	}
}

void
Mixer_UI::hide_strip (MixerStrip* ms)
{
	TreeModel::Children rows = track_model->children();
	TreeModel::Children::iterator i;
	
	for (i = rows.begin(); i != rows.end(); ++i) {
		
		MixerStrip* strip = (*i)[track_columns.strip];
		if (strip == ms) {
			(*i)[track_columns.visible] = false;
			break;
		}
	}
}

gint
Mixer_UI::start_updating ()
{
    fast_screen_update_connection = ARDOUR_UI::instance()->SuperRapidScreenUpdate.connect (mem_fun(*this, &Mixer_UI::fast_update_strips));
    return 0;
}

gint
Mixer_UI::stop_updating ()
{
    fast_screen_update_connection.disconnect();
    return 0;
}

void
Mixer_UI::fast_update_strips ()
{
	if (is_mapped () && session) {
		for (list<MixerStrip *>::iterator i = strips.begin(); i != strips.end(); ++i) {
			(*i)->fast_update ();
		}
	}
}

void
Mixer_UI::set_all_strips_visibility (bool yn)
{
	TreeModel::Children rows = track_model->children();
	TreeModel::Children::iterator i;

	no_track_list_redisplay = true;

	for (i = rows.begin(); i != rows.end(); ++i) {

		TreeModel::Row row = (*i);
		MixerStrip* strip = row[track_columns.strip];
		
		if (strip == 0) {
			continue;
		}
		
		if (strip->route()->master() || strip->route()->control()) {
			continue;
		}

		(*i)[track_columns.visible] = yn;
	}

	no_track_list_redisplay = false;
	redisplay_track_list ();
}


void
Mixer_UI::set_all_audio_visibility (int tracks, bool yn) 
{
        TreeModel::Children rows = track_model->children();
	TreeModel::Children::iterator i;

	no_track_list_redisplay = true;

	for (i = rows.begin(); i != rows.end(); ++i) {
		TreeModel::Row row = (*i);
		MixerStrip* strip = row[track_columns.strip];

		if (strip == 0) {
			continue;
		}

		if (strip->route()->master() || strip->route()->control()) {
			continue;
		}

		boost::shared_ptr<AudioTrack> at = strip->audio_track();

		switch (tracks) {
		case 0:
			(*i)[track_columns.visible] = yn;
			break;
			
		case 1:
			if (at) { /* track */
				(*i)[track_columns.visible] = yn;
			}
			break;
			
		case 2:
			if (!at) { /* bus */
				(*i)[track_columns.visible] = yn;
			}
			break;
		}
	}

	no_track_list_redisplay = false;
	redisplay_track_list ();
}

void
Mixer_UI::hide_all_routes ()
{
	set_all_strips_visibility (false);
}

void
Mixer_UI::show_all_routes ()
{
	set_all_strips_visibility (true);
}

void
Mixer_UI::show_all_audiobus ()
{
	set_all_audio_visibility (2, true);
}
void
Mixer_UI::hide_all_audiobus ()
{
	set_all_audio_visibility (2, false);
}

void
Mixer_UI::show_all_audiotracks()
{
	set_all_audio_visibility (1, true);
}

void
Mixer_UI::hide_all_audiotracks ()
{
	set_all_audio_visibility (1, false);
}

void
Mixer_UI::show_tracks_with_regions_at_playhead ()
{
	boost::shared_ptr<Session::RouteList> const regions = session->get_routes_with_regions_at (session->transport_frame ());
	
	TreeModel::Children rows = track_model->children ();
	for (TreeModel::Children::iterator i = rows.begin(); i != rows.end(); ++i) {
		boost::shared_ptr<Route> route = (*i)[track_columns.route];

		bool found = false;
		for (Session::RouteList::iterator x = (*regions).begin(); x != (*regions).end(); ++x) {
			if ((*x) == route)
				found = true;
		}
				
		(*i)[track_columns.visible] = found;
	}

	strip_redisplay_does_not_sync_order_keys = true;
	redisplay_track_list ();
	strip_redisplay_does_not_sync_order_keys = false;
}

void
Mixer_UI::track_list_reorder (const TreeModel::Path& path, const TreeModel::iterator& iter, int* new_order)
{
	strip_redisplay_does_not_sync_order_keys = true;
	session->set_remote_control_ids();
	redisplay_track_list ();
	strip_redisplay_does_not_sync_order_keys = false;
}

void
Mixer_UI::track_list_change (const Gtk::TreeModel::Path& path,const Gtk::TreeModel::iterator& iter)
{
	// never reset order keys because of a property change
	strip_redisplay_does_not_reset_order_keys = true; 
	session->set_remote_control_ids();
	redisplay_track_list ();
	strip_redisplay_does_not_reset_order_keys = false;
}

void
Mixer_UI::track_list_delete (const Gtk::TreeModel::Path& path)
{
	/* this could require an order sync */
	session->set_remote_control_ids();
	redisplay_track_list ();
}

void
Mixer_UI::redisplay_track_list ()
{
	TreeModel::Children rows = track_model->children();
	TreeModel::Children::iterator i;
	long order;

	if (no_track_list_redisplay) {
		return;
	}

	for (order = 0, i = rows.begin(); i != rows.end(); ++i, ++order) {
		MixerStrip* strip = (*i)[track_columns.strip];

		if (strip == 0) {
			/* we're in the middle of changing a row, don't worry */
			continue;
		}

		bool visible = (*i)[track_columns.visible];
		
		boost::shared_ptr<Route> route = (*i)[track_columns.route];

		if (visible) {
			strip->set_marked_for_display (true);

			if (!strip_redisplay_does_not_reset_order_keys) {
				strip->route()->set_order_key (get_order_key(), order);
			} 

			if (strip->packed()) {

				if (strip->route()->master() || strip->route()->control()) {
					out_packer.reorder_child (*strip, -1);
				} else {
					strip_packer.reorder_child (*strip, -1); /* put at end */
				}

			} else {

				if (strip->route()->master() || strip->route()->control()) {
					out_packer.pack_start (*strip, false, false);
				} else {
					strip_packer.pack_start (*strip, false, false);
				}
				strip->set_packed (true);
				strip->show_all ();
			}

		} else {

			if (strip->route()->master() || strip->route()->control()) {
				/* do nothing, these cannot be hidden */
			} else {
				if (strip->packed()) {
					strip_packer.remove (*strip);
					strip->set_packed (false);
				}
			}
		}
	}
	
	if (!strip_redisplay_does_not_reset_order_keys && !strip_redisplay_does_not_sync_order_keys) {
		session->sync_order_keys (get_order_key());
	}

	// Rebind all of the midi controls automatically
	
	if (auto_rebinding) {
		auto_rebind_midi_controls ();
	}
}

#ifdef GTKOSX
void
Mixer_UI::queue_draw_all_strips ()
{
	TreeModel::Children rows = track_model->children();
	TreeModel::Children::iterator i;
	long order;

	for (order = 0, i = rows.begin(); i != rows.end(); ++i, ++order) {
		MixerStrip* strip = (*i)[track_columns.strip];

		if (strip == 0) {
			continue;
		}

		bool visible = (*i)[track_columns.visible];
		
		if (visible) {
			strip->queue_draw();
		}
	}
}
#endif

void
Mixer_UI::set_auto_rebinding( bool val )
{
	if( val == TRUE )
	{
		auto_rebinding = TRUE;
		Session::AutoBindingOff();
	}
	else
	{
		auto_rebinding = FALSE;
		Session::AutoBindingOn();
	}
}

void 
Mixer_UI::toggle_auto_rebinding() 
{
	set_auto_rebinding (!auto_rebinding);
	auto_rebind_midi_controls();
}

void 
Mixer_UI::auto_rebind_midi_controls () 
{
	TreeModel::Children rows = track_model->children();
	TreeModel::Children::iterator i;
	int pos;

	// Create bindings for all visible strips and remove those that are not visible
	pos = 1;  // 0 is reserved for the master strip
	for (i = rows.begin(); i != rows.end(); ++i) {
		MixerStrip* strip = (*i)[track_columns.strip];
    
		if ( (*i)[track_columns.visible] == true ) {  // add bindings for
			// make the actual binding
			//cout<<"Auto Binding:  Visible Strip Found: "<<strip->name()<<endl;

			int controlValue = pos;
			if( strip->route()->master() ) {
				controlValue = 0;
			}
			else {
				pos++;
			}

			PBD::Controllable::CreateBinding ( strip->solo_button->get_controllable(), controlValue, 0);
			PBD::Controllable::CreateBinding ( strip->mute_button->get_controllable(), controlValue, 1);

			if( strip->is_audio_track() ) {
				PBD::Controllable::CreateBinding ( strip->rec_enable_button->get_controllable(), controlValue, 2);
			}

			PBD::Controllable::CreateBinding ( &(strip->gpm.get_controllable()), controlValue, 3);
			PBD::Controllable::CreateBinding ( strip->panners.get_controllable(), controlValue, 4);

		}
		else {  // Remove any existing binding
			PBD::Controllable::DeleteBinding ( strip->solo_button->get_controllable() );
			PBD::Controllable::DeleteBinding ( strip->mute_button->get_controllable() );

			if( strip->is_audio_track() ) {
				PBD::Controllable::DeleteBinding ( strip->rec_enable_button->get_controllable() );
			}

			PBD::Controllable::DeleteBinding ( &(strip->gpm.get_controllable()) );
			PBD::Controllable::DeleteBinding ( strip->panners.get_controllable() ); // This only takes the first panner if there are multiples...
		}

	} // for
  
}


struct SignalOrderRouteSorter {
    bool operator() (boost::shared_ptr<Route> a, boost::shared_ptr<Route> b) {
	    /* use of ">" forces the correct sort order */
	    return a->order_key (Mixer_UI::get_order_key()) < b->order_key (Mixer_UI::get_order_key());
    }
};

void
Mixer_UI::initial_track_display ()
{
	boost::shared_ptr<Session::RouteList> routes = session->get_routes();
	Session::RouteList copy (*routes);
	SignalOrderRouteSorter sorter;

	copy.sort (sorter);
	
	no_track_list_redisplay = true;

	track_model->clear ();

	add_strip (copy);

	no_track_list_redisplay = false;

	redisplay_track_list ();
}

void
Mixer_UI::show_track_list_menu ()
{
	if (track_menu == 0) {
		build_track_menu ();
	}

	track_menu->popup (1, gtk_get_current_event_time());
}

bool
Mixer_UI::track_display_button_press (GdkEventButton* ev)
{
	if (Keyboard::is_context_menu_event (ev)) {
		show_track_list_menu ();
		return true;
	}

	TreeIter iter;
	TreeModel::Path path;
	TreeViewColumn* column;
	int cellx;
	int celly;
	
	if (!track_display.get_path_at_pos ((int)ev->x, (int)ev->y, path, column, cellx, celly)) {
		return false;
	}

	switch (GPOINTER_TO_UINT (column->get_data (X_("colnum")))) {
	case TRACK_DISPLAY_STRIPS_COLUMN:
		/* allow normal processing to occur */
		return false;

	case TRACK_DISPLAY_SHOW_COLUMN: /* visibility */

		if ((iter = track_model->get_iter (path))) {
			MixerStrip* strip = (*iter)[track_columns.strip];
			if (strip) {

				if (!strip->route()->master() && !strip->route()->control()) {
					bool visible = (*iter)[track_columns.visible];
					(*iter)[track_columns.visible] = !visible;
				}
#ifdef GTKOSX
				track_display.queue_draw();
#endif
			}
		}
		return true;

	default:
		break;
	}

	return false;
}


void
Mixer_UI::build_track_menu ()
{
	using namespace Menu_Helpers;
	using namespace Gtk;

	track_menu = new Menu;
	track_menu->set_name ("ArdourContextMenu");
	MenuList& items = track_menu->items();
	
	items.push_back (MenuElem (_("Show All"), mem_fun(*this, &Mixer_UI::show_all_routes)));
	items.push_back (MenuElem (_("Hide All"), mem_fun(*this, &Mixer_UI::hide_all_routes)));
	items.push_back (MenuElem (_("Show All Audio Tracks"), mem_fun(*this, &Mixer_UI::show_all_audiotracks)));
	items.push_back (MenuElem (_("Hide All Audio Tracks"), mem_fun(*this, &Mixer_UI::hide_all_audiotracks)));
	items.push_back (MenuElem (_("Show All Audio Busses"), mem_fun(*this, &Mixer_UI::show_all_audiobus)));
	items.push_back (MenuElem (_("Hide All Audio Busses"), mem_fun(*this, &Mixer_UI::hide_all_audiobus)));
	items.push_back (MenuElem (_("Hide All Audio Busses"), mem_fun(*this, &Mixer_UI::hide_all_audiobus)));
	items.push_back (MenuElem (_("Show Tracks With Regions Under Playhead"), mem_fun (*this, &Mixer_UI::show_tracks_with_regions_at_playhead)));
}

void
Mixer_UI::strip_name_changed (void* src, MixerStrip* mx)
{
	ENSURE_GUI_THREAD(bind (mem_fun(*this, &Mixer_UI::strip_name_changed), src, mx));
	
	TreeModel::Children rows = track_model->children();
	TreeModel::Children::iterator i;
	
	for (i = rows.begin(); i != rows.end(); ++i) {
		if ((*i)[track_columns.strip] == mx) {
			(*i)[track_columns.text] = mx->route()->name();
			return;
		}
	} 

	error << _("track display list item for renamed strip not found!") << endmsg;
}


void
Mixer_UI::build_mix_group_context_menu ()
{
	using namespace Gtk::Menu_Helpers;

	mix_group_context_menu = new Menu;
	mix_group_context_menu->set_name ("ArdourContextMenu");
	MenuList& items = mix_group_context_menu->items();

	items.push_back (MenuElem (_("Activate All"), mem_fun(*this, &Mixer_UI::activate_all_mix_groups)));
	items.push_back (MenuElem (_("Disable All"), mem_fun(*this, &Mixer_UI::disable_all_mix_groups)));
	items.push_back (SeparatorElem());
	items.push_back (MenuElem (_("Show All"), mem_fun(*this, &Mixer_UI::show_all_mix_groups)));
	items.push_back (MenuElem (_("Hide All"), mem_fun(*this, &Mixer_UI::hide_all_mix_groups)));
	items.push_back (SeparatorElem());
	items.push_back (MenuElem (_("Add group"), mem_fun(*this, &Mixer_UI::new_mix_group)));
	
}

bool
Mixer_UI::group_display_button_press (GdkEventButton* ev)
{
	if (Keyboard::is_context_menu_event (ev)) {
		if (mix_group_context_menu == 0) {
			build_mix_group_context_menu ();
		}
		mix_group_context_menu->popup (1, ev->time);
		return true;
	}


        RouteGroup* group;
	TreeIter iter;
	TreeModel::Path path;
	TreeViewColumn* column;
	int cellx;
	int celly;

	if (!group_display.get_path_at_pos ((int)ev->x, (int)ev->y, path, column, cellx, celly)) {
		return false;
	}

	switch (GPOINTER_TO_UINT (column->get_data (X_("colnum")))) {
	case GROUP_DISPLAY_GROUP_COLUMN:
		if (Keyboard::is_edit_event (ev)) {
			if ((iter = group_model->get_iter (path))) {
				if ((group = (*iter)[group_columns.group]) != 0) {
					// edit_mix_group (group);
#ifdef GTKOSX
					group_display.queue_draw();
#endif
					return true;
				}
			}
			
		} 
		break;

	case GROUP_DISPLAY_ACTIVE_COLUMN:
		if ((iter = group_model->get_iter (path))) {
			bool active = (*iter)[group_columns.active];
			(*iter)[group_columns.active] = !active;
#ifdef GTKOSX
			group_display.queue_draw();
#endif
			return true;
		}
		break;
		
	case GROUP_DISPLAY_SHOW_COLUMN:
		if ((iter = group_model->get_iter (path))) {
			bool visible = (*iter)[group_columns.visible];
			(*iter)[group_columns.visible] = !visible;
#ifdef GTKOSX
			group_display.queue_draw();
#endif
			return true;
		}
		break;

	default:
		break;
	}
	
	return false;
 }

void
Mixer_UI::activate_all_mix_groups ()
{
        Gtk::TreeModel::Children children = group_model->children();
	for(Gtk::TreeModel::Children::iterator iter = children.begin(); iter != children.end(); ++iter) {
	        (*iter)[group_columns.active] = true;
	}
}



void
Mixer_UI::disable_all_mix_groups ()
{
        Gtk::TreeModel::Children children = group_model->children();
	for(Gtk::TreeModel::Children::iterator iter = children.begin(); iter != children.end(); ++iter) {
	        (*iter)[group_columns.active] = false;
	}
}

void
Mixer_UI::show_all_mix_groups ()
{
       Gtk::TreeModel::Children children = group_model->children();
	for(Gtk::TreeModel::Children::iterator iter = children.begin(); iter != children.end(); ++iter) {
	        (*iter)[group_columns.visible] = true;
	}
}

void
Mixer_UI::hide_all_mix_groups ()
{
       Gtk::TreeModel::Children children = group_model->children();
	for(Gtk::TreeModel::Children::iterator iter = children.begin(); iter != children.end(); ++iter) {
	        (*iter)[group_columns.visible] = false;
	}
}

void
Mixer_UI::mix_groups_changed ()
{
	ENSURE_GUI_THREAD (mem_fun (*this, &Mixer_UI::mix_groups_changed));

	/* just rebuild the while thing */

	group_model->clear ();

	session->foreach_mix_group (mem_fun (*this, &Mixer_UI::add_mix_group));
}

void
Mixer_UI::new_mix_group ()
{
	session->add_mix_group ("");
}

void
Mixer_UI::remove_selected_mix_group ()
{
	Glib::RefPtr<TreeSelection> selection = group_display.get_selection();
	TreeView::Selection::ListHandle_Path rows = selection->get_selected_rows ();

	if (rows.empty()) {
		return;
	}

	TreeView::Selection::ListHandle_Path::iterator i = rows.begin();
	TreeIter iter;
	
	/* selection mode is single, so rows.begin() is it */

	if ((iter = group_model->get_iter (*i))) {

		RouteGroup* rg = (*iter)[group_columns.group];

		if (rg) {
			session->remove_mix_group (*rg);
		}
	}
}

void
Mixer_UI::group_flags_changed (void* src, RouteGroup* group)
{
	if (in_group_row_change) {
		return;
	}

	ENSURE_GUI_THREAD(bind (mem_fun(*this, &Mixer_UI::group_flags_changed), src, group));

	/* force an update of any mixer strips that are using this group,
	   otherwise mix group names don't change in mixer strips 
	*/

	for (list<MixerStrip *>::iterator i = strips.begin(); i != strips.end(); ++i) {
		if ((*i)->mix_group() == group) {
			(*i)->mix_group_changed(0);
		}
	}
	
	TreeModel::iterator i;
	TreeModel::Children rows = group_model->children();
	Glib::RefPtr<TreeSelection> selection = group_display.get_selection();

	in_group_row_change = true;
	
	for (i = rows.begin(); i != rows.end(); ++i) {
		if ((*i)[group_columns.group] == group) {
			(*i)[group_columns.visible] = !group->is_hidden ();
			(*i)[group_columns.active] = group->is_active ();
			(*i)[group_columns.text] = group->name ();
			break;
		}
	}

	in_group_row_change = false;
}

void
Mixer_UI::mix_group_name_edit (const std::string& path, const std::string& new_text)
{
	RouteGroup* group;
	TreeIter iter;

	if ((iter = group_model->get_iter (path))) {
	
		if ((group = (*iter)[group_columns.group]) == 0) {
			return;
		}
		
		if (new_text != group->name()) {
			group->set_name (new_text);
		}
	}
}

void 
Mixer_UI::mix_group_row_change (const Gtk::TreeModel::Path& path,const Gtk::TreeModel::iterator& iter)
{
	RouteGroup* group;

	if (in_group_row_change) {
		return;
	}

	if ((group = (*iter)[group_columns.group]) == 0) {
		return;
	}

	if ((*iter)[group_columns.visible]) {
		for (list<MixerStrip *>::iterator i = strips.begin(); i != strips.end(); ++i) {
			if ((*i)->mix_group() == group) {
				show_strip (*i);
			}
		}
	} else {
		for (list<MixerStrip *>::iterator i = strips.begin(); i != strips.end(); ++i) {
			if ((*i)->mix_group() == group) {
				hide_strip (*i);
			}
		}
	} 

	bool active = (*iter)[group_columns.active];
	group->set_active (active, this);

	std::string name = (*iter)[group_columns.text];

	if (name != group->name()) {
		group->set_name (name);
	}

}

void
Mixer_UI::add_mix_group (RouteGroup* group)

{
	ENSURE_GUI_THREAD(bind (mem_fun(*this, &Mixer_UI::add_mix_group), group));
	bool focus = false;

	in_group_row_change = true;

	TreeModel::Row row = *(group_model->append());
	row[group_columns.active] = group->is_active();

	row[group_columns.visible] = false;

	for (list<MixerStrip *>::iterator i = strips.begin(); i != strips.end(); ++i) {
		if ((*i)->mix_group() == group) {
			if ((*i)->marked_for_display()) {
				row[group_columns.visible] = true;
			}
			break;
		}
	}

	row[group_columns.group] = group;
	if (!group->name().empty()) {
		row[group_columns.text] = group->name();
	} else {
		row[group_columns.text] = _("unnamed");
		focus = true;
	}

	group->FlagsChanged.connect (bind (mem_fun(*this, &Mixer_UI::group_flags_changed), group));
	
	if (focus) {
		TreeViewColumn* col = group_display.get_column (GROUP_DISPLAY_GROUP_COLUMN);
		CellRendererText* name_cell = dynamic_cast<CellRendererText*>(group_display.get_column_cell_renderer (GROUP_DISPLAY_GROUP_COLUMN));
		group_display.set_cursor (group_model->get_path (row), *col, *name_cell, true);
	}

	in_group_row_change = false;
}

bool
Mixer_UI::strip_scroller_button_release (GdkEventButton* ev)
{
	using namespace Menu_Helpers;

	if (Keyboard::is_context_menu_event (ev)) {
		ARDOUR_UI::instance()->add_route (this);
		return true;
	}

	return false;
}

void
Mixer_UI::set_strip_width (Width w)
{
	_strip_width = w;

	for (list<MixerStrip*>::iterator i = strips.begin(); i != strips.end(); ++i) {
		(*i)->set_width (w, this);
	}
}

void
Mixer_UI::set_window_pos_and_size ()
{
	resize (m_width, m_height);
	move (m_root_x, m_root_y);
}

	void
Mixer_UI::get_window_pos_and_size ()
{
	get_position(m_root_x, m_root_y);
	get_size(m_width, m_height);
}

int
Mixer_UI::set_state (const XMLNode& node)
{
	const XMLProperty* prop;
	XMLNode* geometry;
	
	m_width = default_width;
	m_height = default_height;
	m_root_x = 1;
	m_root_y = 1;
	
	if ((geometry = find_named_node (node, "geometry")) != 0) {

		XMLProperty* prop;

		if ((prop = geometry->property("x_size")) == 0) {
			prop = geometry->property ("x-size");
		}
		if (prop) {
			m_width = atoi(prop->value());
		}
		if ((prop = geometry->property("y_size")) == 0) {
			prop = geometry->property ("y-size");
		}
		if (prop) {
			m_height = atoi(prop->value());
		}

		if ((prop = geometry->property ("x_pos")) == 0) {
			prop = geometry->property ("x-pos");
		}
		if (prop) {
			m_root_x = atoi (prop->value());
			
		}
		if ((prop = geometry->property ("y_pos")) == 0) {
			prop = geometry->property ("y-pos");
		}
		if (prop) {
			m_root_y = atoi (prop->value());
		}
	}

	set_window_pos_and_size ();

	if ((prop = node.property ("narrow-strips"))) {
		if (string_is_affirmative (prop->value())) {
			set_strip_width (Narrow);
		} else {
			set_strip_width (Wide);
		}
	}

	if ((prop = node.property ("show-mixer"))) {
		if (string_is_affirmative (prop->value())) {
		       _visible = true;
		}
	}

	return 0;
}

XMLNode&
Mixer_UI::get_state (void)
{
	XMLNode* node = new XMLNode ("Mixer");

	if (is_realized()) {
		Glib::RefPtr<Gdk::Window> win = get_window();
	
		get_window_pos_and_size ();

		XMLNode* geometry = new XMLNode ("geometry");
		char buf[32];
		snprintf(buf, sizeof(buf), "%d", m_width);
		geometry->add_property(X_("x_size"), string(buf));
		snprintf(buf, sizeof(buf), "%d", m_height);
		geometry->add_property(X_("y_size"), string(buf));
		snprintf(buf, sizeof(buf), "%d", m_root_x);
		geometry->add_property(X_("x_pos"), string(buf));
		snprintf(buf, sizeof(buf), "%d", m_root_y);
		geometry->add_property(X_("y_pos"), string(buf));
		
		// written only for compatibility, they are not used.
		snprintf(buf, sizeof(buf), "%d", 0);
		geometry->add_property(X_("x_off"), string(buf));
		snprintf(buf, sizeof(buf), "%d", 0);
		geometry->add_property(X_("y_off"), string(buf));

		snprintf(buf,sizeof(buf), "%d",gtk_paned_get_position (static_cast<Paned*>(&rhs_pane1)->gobj()));
		geometry->add_property(X_("mixer_rhs_pane1_pos"), string(buf));
		snprintf(buf,sizeof(buf), "%d",gtk_paned_get_position (static_cast<Paned*>(&list_hpane)->gobj()));
		geometry->add_property(X_("mixer_list_hpane_pos"), string(buf));

		node->add_child_nocopy (*geometry);
	}

	node->add_property ("narrow-strips", _strip_width == Narrow ? "yes" : "no");

	node->add_property ("show-mixer", _visible ? "yes" : "no");

	return *node;
}


void 
Mixer_UI::pane_allocation_handler (Allocation& alloc, Gtk::Paned* which)
{
	int pos;
	XMLProperty* prop = 0;
	char buf[32];
	XMLNode* node = ARDOUR_UI::instance()->mixer_settings();
	XMLNode* geometry;
	int width, height;
	static int32_t done[3] = { 0, 0, 0 };

	width = default_width;
	height = default_height;

	if ((geometry = find_named_node (*node, "geometry")) != 0) {


		if ((prop = geometry->property ("x_size")) == 0) {
			prop = geometry->property ("x-size");
		}
		if (prop) {
			width = atoi (prop->value());
		}
		if ((prop = geometry->property ("y_size")) == 0) {
			prop = geometry->property ("y-size");
		}
		if (prop) {
			height = atoi (prop->value());
		}
	}

	if (which == static_cast<Gtk::Paned*> (&rhs_pane1)) {

		if (done[0]) {
			return;
		}

		if (!geometry || (prop = geometry->property("mixer_rhs_pane1_pos")) == 0) {
			pos = height / 3;
			snprintf (buf, sizeof(buf), "%d", pos);
		} else {
			pos = atoi (prop->value());
		}

		if ((done[0] = GTK_WIDGET(rhs_pane1.gobj())->allocation.height > pos)) {
			rhs_pane1.set_position (pos);
		}

	} else if (which == static_cast<Gtk::Paned*> (&list_hpane)) {

		if (done[2]) {
			return;
		}

		if (!geometry || (prop = geometry->property("mixer_list_hpane_pos")) == 0) {
			pos = 75;
			snprintf (buf, sizeof(buf), "%d", pos);
		} else {
			pos = atoi (prop->value());
		}

		if ((done[2] = GTK_WIDGET(list_hpane.gobj())->allocation.width > pos)) {
			list_hpane.set_position (pos);
		}
	}
}

void
Mixer_UI::scroll_left () 
{
	Adjustment* adj = scroller.get_hscrollbar()->get_adjustment();
	/* stupid GTK: can't rely on clamping across versions */
	scroller.get_hscrollbar()->set_value (max (adj->get_lower(), adj->get_value() - adj->get_step_increment()));
}

void
Mixer_UI::scroll_right ()
{
	Adjustment* adj = scroller.get_hscrollbar()->get_adjustment();
	/* stupid GTK: can't rely on clamping across versions */
	scroller.get_hscrollbar()->set_value (min (adj->get_upper(), adj->get_value() + adj->get_step_increment()));
}

bool
Mixer_UI::on_key_press_event (GdkEventKey* ev)
{
	switch (ev->keyval) {
	case GDK_Left:
		scroll_left ();
		return true;

	case GDK_Right:
		scroll_right ();
		return true;

	default:
		break;
	}

	return key_press_focus_accelerator_handler (*this, ev);
}

bool
Mixer_UI::on_scroll_event (GdkEventScroll* ev)
{
	switch (ev->direction) {
	case GDK_SCROLL_LEFT:
		scroll_left ();
		return true;
	case GDK_SCROLL_UP:
		if (ev->state & Keyboard::TertiaryModifier) {
			scroll_left ();
			return true;
		}
		return false;

	case GDK_SCROLL_RIGHT:
		scroll_right ();
		return true;

	case GDK_SCROLL_DOWN:
		if (ev->state & Keyboard::TertiaryModifier) {
			scroll_right ();
			return true;
		}
		return false;
	}

	return false;
}
