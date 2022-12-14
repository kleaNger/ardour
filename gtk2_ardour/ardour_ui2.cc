/*
    Copyright (C) 1999 Paul Davis 

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

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include <cmath>

#include <sigc++/bind.h>
#include <pbd/error.h>
#include <pbd/basename.h>
#include <pbd/fastlog.h>
#include <gtkmm2ext/utils.h>
#include <gtkmm2ext/click_box.h>
#include <gtkmm2ext/tearoff.h>

#include <ardour/audioengine.h>
#include <ardour/ardour.h>
#include <ardour/profile.h>
#include <ardour/route.h>

#include "ardour_ui.h"
#include "keyboard.h"
#include "public_editor.h"
#include "audio_clock.h"
#include "actions.h"
#include "utils.h"
#include "theme_manager.h"

#include "i18n.h"

using namespace std;
using namespace ARDOUR;
using namespace PBD;
using namespace Gtkmm2ext;
using namespace Gtk;
using namespace Glib;
using namespace sigc;

int	
ARDOUR_UI::setup_windows ()
{
	if (create_editor ()) {
		error << _("UI: cannot setup editor") << endmsg;
		return -1;
	}

	if (create_mixer ()) {
		error << _("UI: cannot setup mixer") << endmsg;
		return -1;
	}

	/* all other dialogs are created conditionally */

	we_have_dependents ();

	setup_clock ();
	setup_transport();
	build_menu_bar ();

	theme_manager->signal_unmap().connect (bind (sigc::ptr_fun(&ActionManager::uncheck_toggleaction), X_("<Actions>/Common/ToggleThemeManager")));

#ifdef TOP_MENUBAR
	HBox* status_bar_packer = manage (new HBox);
	EventBox* status_bar_event_box = manage (new EventBox);

	status_bar_event_box->add (status_bar_label);
	status_bar_event_box->add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK);
	status_bar_label.set_size_request (300, -1);
	status_bar_packer->pack_start (*status_bar_event_box, true, true, 6);
	status_bar_packer->pack_start (error_log_button, false, false);

	status_bar_label.show ();
	status_bar_event_box->show ();
	status_bar_packer->show ();
	error_log_button.show ();

	error_log_button.signal_clicked().connect (mem_fun (*this, &UI::toggle_errors));
	status_bar_event_box->signal_button_press_event().connect (mem_fun (*this, &ARDOUR_UI::status_bar_button_press));

	editor->get_status_bar_packer().pack_start (*status_bar_packer, true, true);
	editor->get_status_bar_packer().pack_start (menu_bar_base, false, false, 6);
#else
	top_packer.pack_start (menu_bar_base, false, false);
#endif 
	top_packer.pack_start (transport_frame, false, false);

	editor->add_toplevel_controls (top_packer);

	return 0;
}

bool
ARDOUR_UI::status_bar_button_press (GdkEventButton* ev)
{
	bool handled = false;

	switch (ev->button) {
	case 1:
		status_bar_label.set_text ("");
		handled = true;
		break;
	default:
		break;
	}

	return handled;
}

void
ARDOUR_UI::display_message (const char *prefix, gint prefix_len, RefPtr<TextBuffer::Tag> ptag, RefPtr<TextBuffer::Tag> mtag, const char *msg)
{
	string text;

	UI::display_message (prefix, prefix_len, ptag, mtag, msg);
#ifdef TOP_MENUBAR

	if (strcmp (prefix, _("[ERROR]: ")) == 0) {
		text = "<span color=\"red\" weight=\"bold\">";
	} else if (strcmp (prefix, _("[WARNING]: ")) == 0) {
		text = "<span color=\"yellow\" weight=\"bold\">";
	} else if (strcmp (prefix, _("[INFO]: ")) == 0) {
		text = "<span color=\"green\" weight=\"bold\">";
	} else {
		text = "<span color=\"blue\" weight=\"bold\">???";
	}

	text += prefix;
	text += "</span>";
	text += msg;

	status_bar_label.set_markup (text);
#endif
}

void
ARDOUR_UI::setup_transport ()
{
	transport_tearoff = manage (new TearOff (transport_tearoff_hbox));
	transport_tearoff->set_name ("TransportBase");
	transport_tearoff->tearoff_window().signal_key_press_event().connect (bind (sigc::ptr_fun (relay_key_press), &transport_tearoff->tearoff_window()));

	if (Profile->get_sae()) {
		transport_tearoff->set_can_be_torn_off (false);
	}

	transport_hbox.pack_start (*transport_tearoff, true, false);

	transport_base.set_name ("TransportBase");
	transport_base.add (transport_hbox);

	transport_frame.set_shadow_type (SHADOW_OUT);
	transport_frame.set_name ("BaseFrame");
	transport_frame.add (transport_base);

	transport_tearoff->Detach.connect (bind (mem_fun(*this, &ARDOUR_UI::detach_tearoff), static_cast<Box*>(&top_packer), 
						 static_cast<Widget*>(&transport_frame)));
	transport_tearoff->Attach.connect (bind (mem_fun(*this, &ARDOUR_UI::reattach_tearoff), static_cast<Box*> (&top_packer), 
						 static_cast<Widget*> (&transport_frame), 1));
	transport_tearoff->Hidden.connect (bind (mem_fun(*this, &ARDOUR_UI::detach_tearoff), static_cast<Box*>(&top_packer), 
						 static_cast<Widget*>(&transport_frame)));
	transport_tearoff->Visible.connect (bind (mem_fun(*this, &ARDOUR_UI::reattach_tearoff), static_cast<Box*> (&top_packer), 
						  static_cast<Widget*> (&transport_frame), 1));
	
	shuttle_box.set_name ("TransportButton");
	goto_start_button.set_name ("TransportButton");
	goto_end_button.set_name ("TransportButton");
	roll_button.set_name ("TransportButton");
	stop_button.set_name ("TransportButton");
	play_selection_button.set_name ("TransportButton");
	rec_button.set_name ("TransportRecButton");
	auto_loop_button.set_name ("TransportButton");

	auto_return_button.set_name ("TransportButton");
	auto_play_button.set_name ("TransportButton");
	auto_input_button.set_name ("TransportButton");
	punch_in_button.set_name ("TransportButton");
	punch_out_button.set_name ("TransportButton");
	click_button.set_name ("TransportButton");
	time_master_button.set_name ("TransportButton");
	sync_option_combo.set_name ("TransportButton");

	stop_button.set_size_request(29, -1);
	roll_button.set_size_request(29, -1);
	auto_loop_button.set_size_request(29, -1);
	play_selection_button.set_size_request(29, -1);
	goto_start_button.set_size_request(29, -1);
	goto_end_button.set_size_request(29, -1);
	rec_button.set_size_request(29, -1);
	
	Widget* w;

	stop_button.set_visual_state (1);
	
	w = manage (new Image (get_icon (X_("transport_start"))));
	w->show();
	goto_start_button.add (*w);
	w = manage (new Image (get_icon (X_("transport_end"))));
	w->show();
	goto_end_button.add (*w);
	w = manage (new Image (get_icon (X_("transport_play"))));
	w->show();
	roll_button.add (*w);
	w = manage (new Image (get_icon (X_("transport_stop"))));
	w->show();
	stop_button.add (*w);
	w = manage (new Image (get_icon (X_("transport_range"))));
	w->show();
	play_selection_button.add (*w);
	w = manage (new Image (get_icon (X_("transport_record"))));
	w->show();
	rec_button.add (*w);
	w = manage (new Image (get_icon (X_("transport_loop"))));
	w->show();
	auto_loop_button.add (*w);

	RefPtr<Action> act;

	act = ActionManager::get_action (X_("Transport"), X_("Stop"));
	act->connect_proxy (stop_button);
	act = ActionManager::get_action (X_("Transport"), X_("Roll"));
	act->connect_proxy (roll_button);
	act = ActionManager::get_action (X_("Transport"), X_("Record"));
	act->connect_proxy (rec_button);
	act = ActionManager::get_action (X_("Transport"), X_("GotoStart"));
	act->connect_proxy (goto_start_button);
	act = ActionManager::get_action (X_("Transport"), X_("GotoEnd"));
	act->connect_proxy (goto_end_button);
	act = ActionManager::get_action (X_("Transport"), X_("Loop"));
	act->connect_proxy (auto_loop_button);
	act = ActionManager::get_action (X_("Transport"), X_("PlaySelection"));
	act->connect_proxy (play_selection_button);
	act = ActionManager::get_action (X_("Transport"), X_("ToggleTimeMaster"));
	act->connect_proxy (time_master_button);

	ARDOUR_UI::instance()->tooltips().set_tip (roll_button, _("Play from playhead"));
	ARDOUR_UI::instance()->tooltips().set_tip (stop_button, _("Stop playback"));
	ARDOUR_UI::instance()->tooltips().set_tip (play_selection_button, _("Play range/selection"));
	ARDOUR_UI::instance()->tooltips().set_tip (goto_start_button, _("Go to start of session"));
	ARDOUR_UI::instance()->tooltips().set_tip (goto_end_button, _("Go to end of session"));
	ARDOUR_UI::instance()->tooltips().set_tip (auto_loop_button, _("Play loop range"));

	ARDOUR_UI::instance()->tooltips().set_tip (auto_return_button, _("Return to last playback start when stopped"));
	ARDOUR_UI::instance()->tooltips().set_tip (auto_play_button, _("Start playback after any locate"));
	ARDOUR_UI::instance()->tooltips().set_tip (auto_input_button, _("Be sensible about input monitoring"));
	ARDOUR_UI::instance()->tooltips().set_tip (punch_in_button, _("Start recording at auto-punch start"));
	ARDOUR_UI::instance()->tooltips().set_tip (punch_out_button, _("Stop recording at auto-punch end"));
	ARDOUR_UI::instance()->tooltips().set_tip (click_button, _("Enable/Disable audio click"));
	ARDOUR_UI::instance()->tooltips().set_tip (sync_option_combo, _("Positional sync source"));
	ARDOUR_UI::instance()->tooltips().set_tip (time_master_button, string_compose (_("Does %1 control the time?"), PROGRAM_NAME));
	ARDOUR_UI::instance()->tooltips().set_tip (shuttle_box, _("Shuttle speed control"));
	ARDOUR_UI::instance()->tooltips().set_tip (shuttle_units_button, _("Select semitones or %%-age for speed display"));
	ARDOUR_UI::instance()->tooltips().set_tip (speed_display_box, _("Current transport speed"));
	
	shuttle_box.set_flags (CAN_FOCUS);
	shuttle_box.add_events (Gdk::ENTER_NOTIFY_MASK|Gdk::LEAVE_NOTIFY_MASK|Gdk::BUTTON_RELEASE_MASK|Gdk::BUTTON_PRESS_MASK|Gdk::POINTER_MOTION_MASK|Gdk::SCROLL_MASK);
	shuttle_box.set_size_request (100, 15);

	shuttle_box.signal_button_press_event().connect (mem_fun(*this, &ARDOUR_UI::shuttle_box_button_press));
	shuttle_box.signal_button_release_event().connect (mem_fun(*this, &ARDOUR_UI::shuttle_box_button_release));
	shuttle_box.signal_scroll_event().connect (mem_fun(*this, &ARDOUR_UI::shuttle_box_scroll));
	shuttle_box.signal_motion_notify_event().connect (mem_fun(*this, &ARDOUR_UI::shuttle_box_motion));
	shuttle_box.signal_expose_event().connect (mem_fun(*this, &ARDOUR_UI::shuttle_box_expose));

	/* clocks, etc. */

	ARDOUR_UI::Clock.connect (bind (mem_fun (primary_clock, &AudioClock::set), 1));
	ARDOUR_UI::Clock.connect (bind (mem_fun (secondary_clock, &AudioClock::set), 2));

	primary_clock.ValueChanged.connect (mem_fun(*this, &ARDOUR_UI::primary_clock_value_changed));
	secondary_clock.ValueChanged.connect (mem_fun(*this, &ARDOUR_UI::secondary_clock_value_changed));
	big_clock.ValueChanged.connect (mem_fun(*this, &ARDOUR_UI::big_clock_value_changed));

	ARDOUR_UI::instance()->tooltips().set_tip (primary_clock, _("Primary clock"));
	ARDOUR_UI::instance()->tooltips().set_tip (secondary_clock, _("secondary clock"));

	ActionManager::get_action ("Transport", "ToggleAutoReturn")->connect_proxy (auto_return_button);
	ActionManager::get_action ("Transport", "ToggleAutoPlay")->connect_proxy (auto_play_button);
	ActionManager::get_action ("Transport", "ToggleAutoInput")->connect_proxy (auto_input_button);
	ActionManager::get_action ("Transport", "ToggleClick")->connect_proxy (click_button);
	ActionManager::get_action ("Transport", "TogglePunchIn")->connect_proxy (punch_in_button);
	ActionManager::get_action ("Transport", "TogglePunchOut")->connect_proxy (punch_out_button);

	preroll_button.set_name ("TransportButton");
	postroll_button.set_name ("TransportButton");

	preroll_clock.set_mode (AudioClock::MinSec);
	preroll_clock.set_name ("TransportClockDisplay");
	postroll_clock.set_mode (AudioClock::MinSec);
	postroll_clock.set_name ("TransportClockDisplay");

	/* alerts */

	/* CANNOT bind these to clicked or toggled, must use pressed or released */

	solo_alert_button.set_name ("TransportSoloAlert");
	solo_alert_button.signal_pressed().connect (mem_fun(*this,&ARDOUR_UI::solo_alert_toggle));
	auditioning_alert_button.set_name ("TransportAuditioningAlert");
	auditioning_alert_button.signal_pressed().connect (mem_fun(*this,&ARDOUR_UI::audition_alert_toggle));

	tooltips().set_tip (solo_alert_button, _("When active, something is soloed.\nClick to de-solo everything"));
	tooltips().set_tip (auditioning_alert_button, _("When active, auditioning is taking place\nClick to stop the audition"));

	alert_box.pack_start (solo_alert_button, false, false);
	alert_box.pack_start (auditioning_alert_button, false, false);

	transport_tearoff_hbox.set_border_width (3);

	transport_tearoff_hbox.pack_start (goto_start_button, false, false);
	transport_tearoff_hbox.pack_start (goto_end_button, false, false);

	Frame* sframe = manage (new Frame);
	VBox*  svbox = manage (new VBox);
	HBox*  shbox = manage (new HBox);

	sframe->set_shadow_type (SHADOW_IN);
	sframe->add (shuttle_box);

	shuttle_box.set_name (X_("ShuttleControl"));

	speed_display_box.add (speed_display_label);
	speed_display_box.set_name (X_("ShuttleDisplay"));
	set_size_request_to_display_given_text (speed_display_label, X_("> 24.0"), 2, 2);

	shuttle_units_button.set_name (X_("ShuttleButton"));
	shuttle_units_button.signal_clicked().connect (mem_fun(*this, &ARDOUR_UI::shuttle_unit_clicked));
	
	shuttle_style_button.set_name (X_("ShuttleStyleButton"));

	vector<string> shuttle_strings;
	shuttle_strings.push_back (_("sprung"));
	shuttle_strings.push_back (_("wheel"));
	set_popdown_strings (shuttle_style_button, shuttle_strings, true);
	shuttle_style_button.signal_changed().connect (mem_fun (*this, &ARDOUR_UI::shuttle_style_changed));

	Frame* sdframe = manage (new Frame);

	sdframe->set_shadow_type (SHADOW_IN);
	sdframe->add (speed_display_box);

	mtc_port_changed ();
	sync_option_combo.signal_changed().connect (mem_fun (*this, &ARDOUR_UI::sync_option_changed));
	// XXX HOW TO USE set_popdown_strings() and combo_fudge with this when we don't know
	// the real strings till later?
	set_size_request_to_display_given_text (sync_option_combo, X_("Igternal"), 4+COMBO_FUDGE, 15);

	shbox->pack_start (*sdframe, false, false);
	shbox->pack_start (shuttle_units_button, true, true);
	shbox->pack_start (shuttle_style_button, false, false);
	
	svbox->pack_start (*sframe, false, false);
	svbox->pack_start (*shbox, false, false);

	transport_tearoff_hbox.pack_start (*svbox, false, false, 3);

	transport_tearoff_hbox.pack_start (auto_loop_button, false, false);
	if (!Profile->get_sae()) {
		transport_tearoff_hbox.pack_start (play_selection_button, false, false);
	}
	transport_tearoff_hbox.pack_start (roll_button, false, false);
	transport_tearoff_hbox.pack_start (stop_button, false, false);
	transport_tearoff_hbox.pack_start (rec_button, false, false, 6);

	HBox* clock_box = manage (new HBox);
	clock_box->pack_start (primary_clock, false, false);
	if (!ARDOUR::Profile->get_small_screen()) {
		clock_box->pack_start (secondary_clock, false, false);
	}

	if (!Profile->get_sae()) {
		VBox* time_controls_box = manage (new VBox);
		time_controls_box->pack_start (sync_option_combo, false, false);
		time_controls_box->pack_start (time_master_button, false, false);
		clock_box->pack_start (*time_controls_box, false, false, 1);
	}

	transport_tearoff_hbox.pack_start (*clock_box, false, false, 0);

	HBox* toggle_box = manage(new HBox);
	
	VBox* punch_box = manage (new VBox);
	punch_box->pack_start (punch_in_button, false, false);
	punch_box->pack_start (punch_out_button, false, false);
	toggle_box->pack_start (*punch_box, false, false);

	VBox* auto_box = manage (new VBox);
	auto_box->pack_start (auto_play_button, false, false);
	auto_box->pack_start (auto_return_button, false, false);
	toggle_box->pack_start (*auto_box, false, false);
	
	VBox* io_box = manage (new VBox);
	io_box->pack_start (auto_input_button, false, false);
	io_box->pack_start (click_button, false, false);
	toggle_box->pack_start (*io_box, false, false);
	
	/* desensitize */

	set_transport_sensitivity (false);

//	toggle_box->pack_start (preroll_button, false, false);
//	toggle_box->pack_start (preroll_clock, false, false);

//	toggle_box->pack_start (postroll_button, false, false);
//	toggle_box->pack_start (postroll_clock, false, false);

	transport_tearoff_hbox.pack_start (*toggle_box, false, false, 4);
	transport_tearoff_hbox.pack_start (alert_box, false, false);

	if (Profile->get_sae()) {
		Image* img = manage (new Image ((::get_icon (X_("sae")))));
		transport_tearoff_hbox.pack_end (*img, false, false, 6);
	}
}

void
ARDOUR_UI::manage_window (Window& win)
{
	win.signal_delete_event().connect (bind (sigc::ptr_fun (just_hide_it), &win));
	win.signal_enter_notify_event().connect (bind (mem_fun (Keyboard::the_keyboard(), &Keyboard::enter_window), &win));
	win.signal_leave_notify_event().connect (bind (mem_fun (Keyboard::the_keyboard(), &Keyboard::leave_window), &win));
}

void
ARDOUR_UI::detach_tearoff (Box* b, Widget* w)
{
//	editor->ensure_float (transport_tearoff->tearoff_window());
	b->remove (*w);
}

void
ARDOUR_UI::reattach_tearoff (Box* b, Widget* w, int32_t n)
{
	b->pack_start (*w);
	b->reorder_child (*w, n);
}

void
ARDOUR_UI::soloing_changed (bool onoff)
{
	if (solo_alert_button.get_active() != onoff) {
		solo_alert_button.set_active (onoff);
	}
}

void
ARDOUR_UI::_auditioning_changed (bool onoff)
{
	if (auditioning_alert_button.get_active() != onoff) {
		auditioning_alert_button.set_active (onoff);
		set_transport_sensitivity (!onoff);
	}
}

void
ARDOUR_UI::auditioning_changed (bool onoff)
{
	UI::instance()->call_slot(bind (mem_fun(*this, &ARDOUR_UI::_auditioning_changed), onoff));
}

void
ARDOUR_UI::audition_alert_toggle ()
{
	if (session) {
		session->cancel_audition();
	}
}

void
ARDOUR_UI::solo_alert_toggle ()
{
	if (session) {
		session->set_all_solo (!session->soloing());
	}
}

void
ARDOUR_UI::solo_blink (bool onoff)
{
	if (session == 0) {
		return;
	}
	
	if (session->soloing()) {
		if (onoff) {
			solo_alert_button.set_state (STATE_ACTIVE);
		} else {
			solo_alert_button.set_state (STATE_NORMAL);
		}
	} else {
		solo_alert_button.set_active (false);
		solo_alert_button.set_state (STATE_NORMAL);
	}
}

void
ARDOUR_UI::audition_blink (bool onoff)
{
	if (session == 0) {
		return;
	}
	
	if (session->is_auditioning()) {
		if (onoff) {
			auditioning_alert_button.set_state (STATE_ACTIVE);
		} else {
			auditioning_alert_button.set_state (STATE_NORMAL);
		}
	} else {
		auditioning_alert_button.set_active (false);
		auditioning_alert_button.set_state (STATE_NORMAL);
	}
}

void
ARDOUR_UI::build_shuttle_context_menu ()
{
	using namespace Menu_Helpers;

	shuttle_context_menu = new Menu();
	MenuList& items = shuttle_context_menu->items();

	Menu* speed_menu = manage (new Menu());
	MenuList& speed_items = speed_menu->items();

	RadioMenuItem::Group group;

	speed_items.push_back (RadioMenuElem (group, "8", bind (mem_fun (*this, &ARDOUR_UI::set_shuttle_max_speed), 8.0f)));
	if (shuttle_max_speed == 8.0) {
		static_cast<RadioMenuItem*>(&speed_items.back())->set_active ();	
	}
	speed_items.push_back (RadioMenuElem (group, "6", bind (mem_fun (*this, &ARDOUR_UI::set_shuttle_max_speed), 6.0f)));
	if (shuttle_max_speed == 6.0) {
		static_cast<RadioMenuItem*>(&speed_items.back())->set_active ();	
	}
	speed_items.push_back (RadioMenuElem (group, "4", bind (mem_fun (*this, &ARDOUR_UI::set_shuttle_max_speed), 4.0f)));
	if (shuttle_max_speed == 4.0) {
		static_cast<RadioMenuItem*>(&speed_items.back())->set_active ();	
	}
	speed_items.push_back (RadioMenuElem (group, "3", bind (mem_fun (*this, &ARDOUR_UI::set_shuttle_max_speed), 3.0f)));
	if (shuttle_max_speed == 3.0) {
		static_cast<RadioMenuItem*>(&speed_items.back())->set_active ();	
	}
	speed_items.push_back (RadioMenuElem (group, "2", bind (mem_fun (*this, &ARDOUR_UI::set_shuttle_max_speed), 2.0f)));
	if (shuttle_max_speed == 2.0) {
		static_cast<RadioMenuItem*>(&speed_items.back())->set_active ();	
	}
	speed_items.push_back (RadioMenuElem (group, "1.5", bind (mem_fun (*this, &ARDOUR_UI::set_shuttle_max_speed), 1.5f)));
	if (shuttle_max_speed == 1.5) {
		static_cast<RadioMenuItem*>(&speed_items.back())->set_active ();	
	}

	items.push_back (MenuElem (_("Maximum speed"), *speed_menu));
}

void
ARDOUR_UI::show_shuttle_context_menu ()
{
	if (shuttle_context_menu == 0) {
		build_shuttle_context_menu ();
	}

	shuttle_context_menu->popup (1, gtk_get_current_event_time());
}

void
ARDOUR_UI::set_shuttle_max_speed (float speed)
{
	shuttle_max_speed = speed;
}

gint
ARDOUR_UI::shuttle_box_button_press (GdkEventButton* ev)
{
	if (!session) {
		return true;
	}

	if (shuttle_controller_binding_proxy.button_press_handler (ev)) {
		return true;
	}

	if (Keyboard::is_context_menu_event (ev)) {
		show_shuttle_context_menu ();
		return true;
	}

	switch (ev->button) {
	case 1:
		shuttle_box.add_modal_grab ();
		shuttle_grabbed = true;
		mouse_shuttle (ev->x, true);
		break;

	case 2:
	case 3:
		return true;
		break;
	}

	return true;
}

gint
ARDOUR_UI::shuttle_box_button_release (GdkEventButton* ev)
{
	if (!session) {
		return true;
	}
	
	switch (ev->button) {
	case 1:
		mouse_shuttle (ev->x, true);
		shuttle_grabbed = false;
		shuttle_box.remove_modal_grab ();
		if (Config->get_shuttle_behaviour() == Sprung) {
			if (Config->get_auto_play() || roll_button.get_visual_state()) {
				shuttle_fract = SHUTTLE_FRACT_SPEED1;				
				session->request_transport_speed (1.0);
				stop_button.set_visual_state (0);
				roll_button.set_visual_state (1);
			} else {
				shuttle_fract = 0;
				session->request_stop ();
			}
			shuttle_box.queue_draw ();
		}
		return true;

	case 2:
		if (session->transport_rolling()) {
			shuttle_fract = SHUTTLE_FRACT_SPEED1;
			session->request_transport_speed (1.0);
			stop_button.set_visual_state (0);
			roll_button.set_visual_state (1);
		} else {
			shuttle_fract = 0;
		}
		shuttle_box.queue_draw ();
		return true;

	case 3:
	default:
		return true;

	}

	use_shuttle_fract (true);

	return true;
}

gint
ARDOUR_UI::shuttle_box_scroll (GdkEventScroll* ev)
{
	if (!session) {
		return true;
	}
	
	switch (ev->direction) {
		
	case GDK_SCROLL_UP:
		shuttle_fract += 0.005;
		break;
	case GDK_SCROLL_DOWN:
		shuttle_fract -= 0.005;
		break;
	default:
		/* scroll left/right */
		return false;
	}

	use_shuttle_fract (true);

	return true;
}

gint
ARDOUR_UI::shuttle_box_motion (GdkEventMotion* ev)
{
	if (!session || !shuttle_grabbed) {
		return true;
	}

	return mouse_shuttle (ev->x, false);
}

gint
ARDOUR_UI::mouse_shuttle (double x, bool force)
{
	double half_width = shuttle_box.get_width() / 2.0;
	double distance = x - half_width;

	if (distance > 0) {
		distance = min (distance, half_width);
	} else {
		distance = max (distance, -half_width);
	}

	shuttle_fract = distance / half_width;
	use_shuttle_fract (force);
	return true;
}

void
ARDOUR_UI::set_shuttle_fract (double f)
{
	shuttle_fract = f;
	use_shuttle_fract (false);
}

void
ARDOUR_UI::use_shuttle_fract (bool force)
{
	microseconds_t now = get_microseconds();
	
	/* do not attempt to submit a motion-driven transport speed request
	   more than once per process cycle.
	 */

	if (!force && (last_shuttle_request - now) < (microseconds_t) engine->usecs_per_cycle()) {
		return;
	}
	
	last_shuttle_request = now;

	if (Config->get_shuttle_units() == Semitones) {

		const double step = 1.0 / 24.0; // range is 24 semitones up & down
		double semitones;
		double speed;

		semitones = round (shuttle_fract / step);
		speed = pow (2.0, (semitones / 12.0));

		session->request_transport_speed (speed);

	} else {

		bool neg;
		double fract;
		
		neg = (shuttle_fract < 0.0);

		fract = 1 - sqrt (1 - (shuttle_fract * shuttle_fract)); // Formula A1

		if (neg) {
			fract = -fract;
		}

		session->request_transport_speed (shuttle_max_speed * fract);
	}

	shuttle_box.queue_draw ();
}

gint
ARDOUR_UI::shuttle_box_expose (GdkEventExpose* event)
{
	gint x;
	Glib::RefPtr<Gdk::Window> win (shuttle_box.get_window());

	/* redraw the background */

	win->draw_rectangle (shuttle_box.get_style()->get_bg_gc (shuttle_box.get_state()),
			     true,
			     event->area.x, event->area.y,
			     event->area.width, event->area.height);


	x = (gint) floor ((shuttle_box.get_width() / 2.0) + (0.5 * (shuttle_box.get_width() * shuttle_fract)));

	/* draw line */

	win->draw_line (shuttle_box.get_style()->get_fg_gc (shuttle_box.get_state()),
			x,
			0,
			x,
			shuttle_box.get_height());
	return true;
}

void
ARDOUR_UI::shuttle_unit_clicked ()
{
	if (shuttle_unit_menu == 0) {
		shuttle_unit_menu = dynamic_cast<Menu*> (ActionManager::get_widget ("/ShuttleUnitPopup"));
	}
	shuttle_unit_menu->popup (1, gtk_get_current_event_time());
}

void
ARDOUR_UI::shuttle_style_changed ()
{
	string str = shuttle_style_button.get_active_text ();

	if (str == _("sprung")) {
		Config->set_shuttle_behaviour (Sprung);
	} else if (str == _("wheel")) {
		Config->set_shuttle_behaviour (Wheel);
	}
}

void
ARDOUR_UI::update_speed_display ()
{
	if (!session) {
		if (last_speed_displayed != 0) {
			speed_display_label.set_text (_("stop"));
			last_speed_displayed = 0;
		}
		return;
	}

	char buf[32];
	float x = session->transport_speed ();

	if (x != last_speed_displayed) {

		if (x != 0) {
			if (Config->get_shuttle_units() == Percentage) {
				snprintf (buf, sizeof (buf), "%.2f", x);
			} else {

				if (x < 0) {
					snprintf (buf, sizeof (buf), "< %d", (int) round (12.0 * fast_log2 (-x)));
				} else {
					snprintf (buf, sizeof (buf), "> %d", (int) round (12.0 * fast_log2 (x)));
				}
			}
			speed_display_label.set_text (buf);
		} else {
			speed_display_label.set_text (_("stop"));
		}

		last_speed_displayed = x;
	}
}	
	
void
ARDOUR_UI::set_transport_sensitivity (bool yn)
{
	ActionManager::set_sensitive (ActionManager::transport_sensitive_actions, yn);
	shuttle_box.set_sensitive (yn);
}

void
ARDOUR_UI::editor_realized ()
{
	Config->map_parameters (mem_fun (*this, &ARDOUR_UI::parameter_changed));

	set_size_request_to_display_given_text (speed_display_box, _("-0.55"), 2, 2);
	reset_dpi();
}

void
ARDOUR_UI::sync_option_changed ()
{
	if (session) {
		string txt = sync_option_combo.get_active_text ();
		if (txt.length()) {
			session->request_slave_source (string_to_slave_source (txt));
		}
	}
}

void
ARDOUR_UI::maximise_editing_space ()
{
	if (!editor) {
		return;
	}

	transport_tearoff->set_visible (false);
	editor->maximise_editing_space ();
}

void
ARDOUR_UI::restore_editing_space ()
{
	if (!editor) {
		return;
	}

	transport_tearoff->set_visible (true);
	editor->restore_editing_space ();
}
