/*
    Copyright (C) 2003-2004 Paul Davis 

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

#include <glibmm/miscutils.h>
#include <gtkmm2ext/utils.h>
#include <gtkmm2ext/window_title.h>

#include <pbd/enumwriter.h>

#include <ardour/audioengine.h>

#include "editor.h"
#include "mixer_strip.h"
#include "ardour_ui.h"
#include "selection.h"
#include "audio_time_axis.h"
#include "automation_time_axis.h"
#include "actions.h"

#include "i18n.h"

using namespace Gtkmm2ext;
using namespace PBD;

void
Editor::editor_mixer_button_toggled ()
{
	Glib::RefPtr<Gtk::Action> act = ActionManager::get_action (X_("Editor"), X_("show-editor-mixer"));
	if (act) {
		Glib::RefPtr<Gtk::ToggleAction> tact = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(act);
		show_editor_mixer (tact->get_active());
	}
}

void
Editor::cms_deleted ()
{
	current_mixer_strip = 0;
}

void
Editor::show_editor_mixer (bool yn)
{
	boost::shared_ptr<ARDOUR::Route> r;

	show_editor_mixer_when_tracks_arrive = false;

	if (!session) {
		show_editor_mixer_when_tracks_arrive = yn;
		return;
	}

	if (yn) {

		if (selection->tracks.empty()) {
			
			if (track_views.empty()) {	
				show_editor_mixer_when_tracks_arrive = true;
				return;
			} 

			for (TrackViewList::iterator i = track_views.begin(); i != track_views.end(); ++i) {
				AudioTimeAxisView* atv;
				
				if ((atv = dynamic_cast<AudioTimeAxisView*> (*i)) != 0) {
					r = atv->route();
					break;
				}
			}

		} else {

			sort_track_selection ();
			
			for (TrackSelection::iterator i = selection->tracks.begin(); i != selection->tracks.end(); ++i) {
				AudioTimeAxisView* atv;
				
				if ((atv = dynamic_cast<AudioTimeAxisView*> (*i)) != 0) {
					r = atv->route();
					break;
				}
			}
		}

		if (r) {
			bool created;

			if (current_mixer_strip == 0) {
				create_editor_mixer ();
				created = true;
			} else {
				created = false;
			}

			current_mixer_strip->set_route (r);

			if (created) {
				current_mixer_strip->set_width (editor_mixer_strip_width, (void*) this);
			}
		}
		
		if (current_mixer_strip->get_parent() == 0) {
			global_hpacker.pack_start (*current_mixer_strip, Gtk::PACK_SHRINK );
 			global_hpacker.reorder_child (*current_mixer_strip, 0);
			current_mixer_strip->show_all ();
		}

	} else {

		if (current_mixer_strip) {
			if (current_mixer_strip->get_parent() != 0) {
				global_hpacker.remove (*current_mixer_strip);
			}
		}
	}

#ifdef GTKOSX
	/* XXX gtk problem here */
	ensure_all_elements_drawn();
#endif
}

#ifdef GTKOSX
void
Editor::ensure_all_elements_drawn ()
{
	controls_layout.queue_draw ();
	ruler_label_event_box.queue_draw ();
	time_button_event_box.queue_draw ();
}
#endif

void
Editor::create_editor_mixer ()
{
	current_mixer_strip = new MixerStrip (*ARDOUR_UI::instance()->the_mixer(),
					      *session,
					      false);
	current_mixer_strip->Hiding.connect (mem_fun(*this, &Editor::current_mixer_strip_hidden));
	current_mixer_strip->GoingAway.connect (mem_fun(*this, &Editor::current_mixer_strip_removed));
#ifdef GTKOSX
	current_mixer_strip->WidthChanged.connect (mem_fun(*this, &Editor::ensure_all_elements_drawn));
#endif
	current_mixer_strip->set_embedded (true);
}	

void
Editor::set_selected_mixer_strip (TimeAxisView& view)
{
	bool show = false;
	bool created;

	if (!session)
		return;

	Glib::RefPtr<Gtk::Action> act = ActionManager::get_action (X_("Editor"), X_("show-editor-mixer"));
	if (act) {
		Glib::RefPtr<Gtk::ToggleAction> tact = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(act);
		if (!tact || !tact->get_active()) {
			/* not showing mixer strip presently */
			return;
		}
	}

	if (current_mixer_strip == 0) {
		create_editor_mixer ();
		created = true;
	} else {
		created = false;
	}

 	//if this is an automation track then we should show the parent
	boost::shared_ptr<ARDOUR::Route> route;
	AutomationTimeAxisView *auto_tav;
	if ( (auto_tav  = dynamic_cast<AutomationTimeAxisView*>(&view)) == 0 ) {
		AudioTimeAxisView* at = dynamic_cast<AudioTimeAxisView*>(&view);
		if (at != NULL)
			route = at->route();
	} else {
		AudioTimeAxisView *parent = dynamic_cast<AudioTimeAxisView*>( view.get_parent() );
		if (parent != NULL) {
			route = parent->route();
		}
	}
	
	if (current_mixer_strip->route() == route) {
		return;
	}
	
	if (current_mixer_strip->get_parent()) {
		show = true;
	}

	current_mixer_strip->set_route (route);

	if (created) {
		current_mixer_strip->set_width (editor_mixer_strip_width, (void*) this);
	}

	if (show) {
		show_editor_mixer (true);
	}
}

double current = 0.0;

void
Editor::update_current_screen ()
{
	if (session && session->engine().running()) {

		nframes64_t frame;

		//frame = session->audible_frame();
		frame = session->transport_frame ();

		if (_dragging_playhead) {
			goto almost_done;
		}

#undef DEBUG_CURRENT_SCREEN
#if DEBUG_CURRENT_SCREEN

		cerr << "@ " << frame << " last " << last_update_frame << " follow " << _follow_playhead 
		     << " ret " << session->requested_return_frame()
		     << " left " << leftmost_frame
		     << " right " << leftmost_frame + current_page_frames()
		     << " speed " << session->transport_speed ()
		     << endl;
#endif

		/* only update if the playhead is on screen or we are following it */

		if (_follow_playhead && session->requested_return_frame() < 0) {

			//playhead_cursor->canvas_item.show();

			if (frame != last_update_frame) {


		if ( !_stationary_playhead ) {
				if (frame < leftmost_frame || frame > leftmost_frame + current_page_frames()) {

#ifdef DEBUG_CURRENT_SCREEN
					cerr << "\trecenter...\n";
#endif 					
					if (session->transport_speed() < 0) {
						if (frame > (current_page_frames()/2)) {
							center_screen (frame-(current_page_frames()/2));
						} else {
							center_screen (current_page_frames()/2);
						}
					} else {
						center_screen (frame+(current_page_frames()/2));
					}
				}

				playhead_cursor->set_position (frame);

		} else {
				
				/* don't do continuous scroll till the new position is in the rightmost quarter of the 
				   editor canvas
				*/
				
				if (session->transport_speed()) {
					double target = ((double)frame - (double)current_page_frames()/2.0) / frames_per_unit;
					if (target <= 0.0) target = 0.0;
					if ( fabs(target - current) < current_page_frames()/frames_per_unit ) {
						target = (target * 0.15) + (current * 0.85);
					} else {
						/* relax */
					}
					//printf("frame: %d,  cpf: %d,  fpu: %6.6f, current: %6.6f, target : %6.6f\n", frame, current_page_frames(), frames_per_unit, current, target );
					current = target;
					horizontal_adjustment.set_value ( current );
				}
				
				playhead_cursor->set_position (frame);

				}

			}

		} else {
			
			if (frame != last_update_frame) {
				playhead_cursor->set_position (frame);
			}
		}

	  almost_done:
		last_update_frame = frame;
		if (current_mixer_strip) {
			current_mixer_strip->fast_update ();
		}
		
	}
}

void
Editor::current_mixer_strip_removed ()
{
	if (current_mixer_strip) {
		/* it is being deleted elsewhere */
		current_mixer_strip = 0;
	}
}

void
Editor::current_mixer_strip_hidden ()
{
	Glib::RefPtr<Gtk::Action> act = ActionManager::get_action (X_("Editor"), X_("show-editor-mixer"));
	if (act) {
		Glib::RefPtr<Gtk::ToggleAction> tact = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(act);
		tact->set_active (false);
	}
}

void
Editor::session_going_away ()
{
	_have_idled = false;
	
	for (vector<sigc::connection>::iterator i = session_connections.begin(); i != session_connections.end(); ++i) {
		(*i).disconnect ();
	}

	stop_scrolling ();
	selection->clear ();
	cut_buffer->clear ();

	clicked_regionview = 0;
	clicked_trackview = 0;
	clicked_audio_trackview = 0;
	clicked_crossfadeview = 0;
	entered_regionview = 0;
	entered_track = 0;
	last_update_frame = 0;
	drag_info.item = 0;

	playhead_cursor->canvas_item.hide ();

	/* hide all tracks */

	hide_all_tracks (false);

	/* rip everything out of the list displays */

	region_list_display.set_model (Glib::RefPtr<Gtk::TreeStore>(0));
	route_list_display.set_model (Glib::RefPtr<Gtk::TreeStore>(0));
	named_selection_display.set_model (Glib::RefPtr<Gtk::TreeStore>(0));
	edit_group_display.set_model (Glib::RefPtr<Gtk::TreeStore>(0));

	region_list_model->clear ();
	route_display_model->clear ();
	named_selection_model->clear ();
	group_model->clear ();

	region_list_display.set_model (region_list_model);
	route_list_display.set_model (route_display_model);
	named_selection_display.set_model (named_selection_model);
	edit_group_display.set_model (group_model);

	edit_point_clock_connection_a.disconnect();
	edit_point_clock_connection_b.disconnect();

	edit_point_clock.set_session (0);
	zoom_range_clock.set_session (0);
	nudge_clock.set_session (0);

	/* clear tempo/meter rulers */
	remove_metric_marks ();
	hide_measures ();
	clear_marker_display ();

	if (current_bbt_points) {
		delete current_bbt_points;
		current_bbt_points = 0;
	}

	/* get rid of any existing editor mixer strip */

	if (current_mixer_strip) {
		if (current_mixer_strip->get_parent() != 0) {
			global_hpacker.remove (*current_mixer_strip);
		}
		delete current_mixer_strip;
		current_mixer_strip = 0;
	}

	WindowTitle title(Glib::get_application_name());
	title += _("Editor");

	set_title (title.get_string());

	session = 0;
}

void
Editor::maybe_add_mixer_strip_width (XMLNode& node)
{
	if (current_mixer_strip) {
		node.add_property ("mixer-width", enum_2_string (current_mixer_strip->get_width()));
	}
}
