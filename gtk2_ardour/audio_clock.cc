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

#include <cstdio> // for sprintf
#include <cmath>

#include <pbd/convert.h>
#include <pbd/enumwriter.h>

#include <gtkmm2ext/utils.h>

#include <ardour/ardour.h>
#include <ardour/session.h>
#include <ardour/tempo.h>
#include <ardour/profile.h>
#include <sigc++/bind.h>

#include "ardour_ui.h"
#include "audio_clock.h"
#include "utils.h"
#include "keyboard.h"
#include "i18n.h"

using namespace ARDOUR;
using namespace PBD;
using namespace sigc;
using namespace Gtk;
using namespace std;

using PBD::atoi;
using PBD::atof;

sigc::signal<void> AudioClock::ModeChanged;
vector<AudioClock*> AudioClock::clocks;

const uint32_t AudioClock::field_length[(int) AudioClock::AudioFrames+1] = {
	2,   /* SMPTE_Hours */
	2,   /* SMPTE_Minutes */
	2,   /* SMPTE_Seconds */
	2,   /* SMPTE_Frames */
	2,   /* MS_Hours */
	2,   /* MS_Minutes */
	5,   /* MS_Seconds */
	3,   /* Bars */
	2,   /* Beats */
	4,   /* Tick */
	10   /* Audio Frame */
};

AudioClock::AudioClock (std::string clock_name, bool transient, std::string widget_name, bool allow_edit, bool duration, bool with_info) 
	: _name (clock_name),
	  is_transient (transient),
	  is_duration (duration),
	  editable (allow_edit),
	  colon1 (":"),
	  colon2 (":"),
	  colon3 (":"),
	  colon4 (":"),
	  colon5 (":"),
	  b1 ("|"),
	  b2 ("|")
{
	session = 0;
	last_when = 0;
	last_pdelta = 0;
	last_sdelta = 0;
	key_entry_state = 0;
	ops_menu = 0;
	dragging = false;
	bbt_reference_time = -1;

	if (with_info) {
		frames_upper_info_label = manage (new Label);
		frames_lower_info_label = manage (new Label);
		smpte_upper_info_label = manage (new Label);
		smpte_lower_info_label = manage (new Label);
		bbt_upper_info_label = manage (new Label);
		bbt_lower_info_label = manage (new Label);
		
		frames_upper_info_label->set_name ("AudioClockFramesUpperInfo");
		frames_lower_info_label->set_name ("AudioClockFramesLowerInfo");
		smpte_upper_info_label->set_name ("AudioClockSMPTEUpperInfo");
		smpte_lower_info_label->set_name ("AudioClockSMPTELowerInfo");
		bbt_upper_info_label->set_name ("AudioClockBBTUpperInfo");
		bbt_lower_info_label->set_name ("AudioClockBBTLowerInfo");

		Gtkmm2ext::set_size_request_to_display_given_text(*smpte_upper_info_label, "23.98",0,0);
		Gtkmm2ext::set_size_request_to_display_given_text(*smpte_lower_info_label, "NDF",0,0);

		Gtkmm2ext::set_size_request_to_display_given_text(*bbt_upper_info_label, "88|88",0,0);
		Gtkmm2ext::set_size_request_to_display_given_text(*bbt_lower_info_label, "888.88",0,0);

		frames_info_box.pack_start (*frames_upper_info_label, true, true);
		frames_info_box.pack_start (*frames_lower_info_label, true, true);
		smpte_info_box.pack_start (*smpte_upper_info_label, true, true);
		smpte_info_box.pack_start (*smpte_lower_info_label, true, true);
		bbt_info_box.pack_start (*bbt_upper_info_label, true, true);
		bbt_info_box.pack_start (*bbt_lower_info_label, true, true);
		
	} else {
		frames_upper_info_label = 0;
		frames_lower_info_label = 0;
		smpte_upper_info_label = 0;
		smpte_lower_info_label = 0;
		bbt_upper_info_label = 0;
		bbt_lower_info_label = 0;
	}	
	
	audio_frames_ebox.add (audio_frames_label);
	
	frames_packer.set_homogeneous (false);
	frames_packer.set_border_width (2);
	frames_packer.pack_start (audio_frames_ebox, false, false);
	
	if (with_info) {
		frames_packer.pack_start (frames_info_box, false, false, 5);
	}
	
	frames_packer_hbox.pack_start (frames_packer, true, false);

	hours_ebox.add (hours_label);
	minutes_ebox.add (minutes_label);
	seconds_ebox.add (seconds_label);
	frames_ebox.add (frames_label);
	bars_ebox.add (bars_label);
	beats_ebox.add (beats_label);
	ticks_ebox.add (ticks_label);
	ms_hours_ebox.add (ms_hours_label);
	ms_minutes_ebox.add (ms_minutes_label);
	ms_seconds_ebox.add (ms_seconds_label);

	smpte_packer.set_homogeneous (false);
	smpte_packer.set_border_width (2);
	smpte_packer.pack_start (hours_ebox, false, false);
	smpte_packer.pack_start (colon1, false, false);
	smpte_packer.pack_start (minutes_ebox, false, false);
	smpte_packer.pack_start (colon2, false, false);
	smpte_packer.pack_start (seconds_ebox, false, false);
	smpte_packer.pack_start (colon3, false, false);
	smpte_packer.pack_start (frames_ebox, false, false);

	if (with_info) {
		smpte_packer.pack_start (smpte_info_box, false, false, 5);
	}

	smpte_packer_hbox.pack_start (smpte_packer, true, false);

	bbt_packer.set_homogeneous (false);
	bbt_packer.set_border_width (2);
	bbt_packer.pack_start (bars_ebox, false, false);
	bbt_packer.pack_start (b1, false, false);
	bbt_packer.pack_start (beats_ebox, false, false);
	bbt_packer.pack_start (b2, false, false);
	bbt_packer.pack_start (ticks_ebox, false, false);

	if (with_info) {
		bbt_packer.pack_start (bbt_info_box, false, false, 5);
	}

	bbt_packer_hbox.pack_start (bbt_packer, true, false);

	minsec_packer.set_homogeneous (false);
	minsec_packer.set_border_width (2);
	minsec_packer.pack_start (ms_hours_ebox, false, false);
	minsec_packer.pack_start (colon4, false, false);
	minsec_packer.pack_start (ms_minutes_ebox, false, false);
	minsec_packer.pack_start (colon5, false, false);
	minsec_packer.pack_start (ms_seconds_ebox, false, false);

	minsec_packer_hbox.pack_start (minsec_packer, true, false);

	clock_frame.set_shadow_type (Gtk::SHADOW_IN);
	clock_frame.set_name ("BaseFrame");

	clock_frame.add (clock_base);

	set_widget_name (widget_name);

	_mode = BBT; /* lie to force mode switch */
	set_mode (SMPTE);

	pack_start (clock_frame, true, true);

	/* the clock base handles button releases for menu popup regardless of
	   editable status. if the clock is editable, the clock base is where
	   we pass focus to after leaving the last editable "field", which
	   will then shutdown editing till the user starts it up again.

	   it does this because the focus out event on the field disables
	   keyboard event handling, and we don't connect anything up to
	   notice focus in on the clock base. hence, keyboard event handling
	   stays disabled.
	*/

	clock_base.add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK|Gdk::SCROLL_MASK);
	clock_base.signal_button_release_event().connect (bind (mem_fun (*this, &AudioClock::field_button_release_event), SMPTE_Hours));

	Session::SMPTEOffsetChanged.connect (mem_fun (*this, &AudioClock::smpte_offset_changed));

	if (editable) {
		setup_events ();
	}

	set (last_when, true);

	if (!is_transient) {
		clocks.push_back (this);
	}
}

void
AudioClock::set_widget_name (string name)
{
	Widget::set_name (name);

	clock_base.set_name (name);

	audio_frames_label.set_name (name);
	hours_label.set_name (name);
	minutes_label.set_name (name);
	seconds_label.set_name (name);
	frames_label.set_name (name);
	bars_label.set_name (name);
	beats_label.set_name (name);
	ticks_label.set_name (name);
	ms_hours_label.set_name (name);
	ms_minutes_label.set_name (name);
	ms_seconds_label.set_name (name);
	hours_ebox.set_name (name);
	minutes_ebox.set_name (name);
	seconds_ebox.set_name (name);
	frames_ebox.set_name (name);
	audio_frames_ebox.set_name (name);
	bars_ebox.set_name (name);
	beats_ebox.set_name (name);
	ticks_ebox.set_name (name);
	ms_hours_ebox.set_name (name);
	ms_minutes_ebox.set_name (name);
	ms_seconds_ebox.set_name (name);
	
	colon1.set_name (name);
	colon2.set_name (name);
	colon3.set_name (name);
	colon4.set_name (name);
	colon5.set_name (name);
	b1.set_name (name);
	b2.set_name (name);

	queue_draw ();
}

void
AudioClock::setup_events ()
{
	clock_base.set_flags (Gtk::CAN_FOCUS);

	hours_ebox.add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK|Gdk::KEY_PRESS_MASK|Gdk::KEY_RELEASE_MASK|Gdk::FOCUS_CHANGE_MASK|Gdk::POINTER_MOTION_MASK|Gdk::SCROLL_MASK);
	minutes_ebox.add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK|Gdk::KEY_PRESS_MASK|Gdk::KEY_RELEASE_MASK|Gdk::FOCUS_CHANGE_MASK|Gdk::POINTER_MOTION_MASK|Gdk::SCROLL_MASK);
	seconds_ebox.add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK|Gdk::KEY_PRESS_MASK|Gdk::KEY_RELEASE_MASK|Gdk::FOCUS_CHANGE_MASK|Gdk::POINTER_MOTION_MASK|Gdk::SCROLL_MASK);
	frames_ebox.add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK|Gdk::KEY_PRESS_MASK|Gdk::KEY_RELEASE_MASK|Gdk::FOCUS_CHANGE_MASK|Gdk::POINTER_MOTION_MASK|Gdk::SCROLL_MASK);
	bars_ebox.add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK|Gdk::KEY_PRESS_MASK|Gdk::KEY_RELEASE_MASK|Gdk::FOCUS_CHANGE_MASK|Gdk::POINTER_MOTION_MASK|Gdk::SCROLL_MASK);
	beats_ebox.add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK|Gdk::KEY_PRESS_MASK|Gdk::KEY_RELEASE_MASK|Gdk::FOCUS_CHANGE_MASK|Gdk::POINTER_MOTION_MASK|Gdk::SCROLL_MASK);
	ticks_ebox.add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK|Gdk::KEY_PRESS_MASK|Gdk::KEY_RELEASE_MASK|Gdk::FOCUS_CHANGE_MASK|Gdk::POINTER_MOTION_MASK|Gdk::SCROLL_MASK);
	ms_hours_ebox.add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK|Gdk::KEY_PRESS_MASK|Gdk::KEY_RELEASE_MASK|Gdk::FOCUS_CHANGE_MASK|Gdk::POINTER_MOTION_MASK|Gdk::SCROLL_MASK);
	ms_minutes_ebox.add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK|Gdk::KEY_PRESS_MASK|Gdk::KEY_RELEASE_MASK|Gdk::FOCUS_CHANGE_MASK|Gdk::POINTER_MOTION_MASK|Gdk::SCROLL_MASK);
	ms_seconds_ebox.add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK|Gdk::KEY_PRESS_MASK|Gdk::KEY_RELEASE_MASK|Gdk::FOCUS_CHANGE_MASK|Gdk::POINTER_MOTION_MASK|Gdk::SCROLL_MASK);
	audio_frames_ebox.add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK|Gdk::KEY_PRESS_MASK|Gdk::KEY_RELEASE_MASK|Gdk::FOCUS_CHANGE_MASK|Gdk::POINTER_MOTION_MASK|Gdk::SCROLL_MASK);

	hours_ebox.set_flags (Gtk::CAN_FOCUS);
	minutes_ebox.set_flags (Gtk::CAN_FOCUS);
	seconds_ebox.set_flags (Gtk::CAN_FOCUS);
	frames_ebox.set_flags (Gtk::CAN_FOCUS);
	audio_frames_ebox.set_flags (Gtk::CAN_FOCUS);
	bars_ebox.set_flags (Gtk::CAN_FOCUS);
	beats_ebox.set_flags (Gtk::CAN_FOCUS);
	ticks_ebox.set_flags (Gtk::CAN_FOCUS);
	ms_hours_ebox.set_flags (Gtk::CAN_FOCUS);
	ms_minutes_ebox.set_flags (Gtk::CAN_FOCUS);
	ms_seconds_ebox.set_flags (Gtk::CAN_FOCUS);

	hours_ebox.signal_motion_notify_event().connect (bind (mem_fun(*this, &AudioClock::field_motion_notify_event), SMPTE_Hours));
	minutes_ebox.signal_motion_notify_event().connect (bind (mem_fun(*this, &AudioClock::field_motion_notify_event), SMPTE_Minutes));
	seconds_ebox.signal_motion_notify_event().connect (bind (mem_fun(*this, &AudioClock::field_motion_notify_event), SMPTE_Seconds));
	frames_ebox.signal_motion_notify_event().connect (bind (mem_fun(*this, &AudioClock::field_motion_notify_event), SMPTE_Frames));
	audio_frames_ebox.signal_motion_notify_event().connect (bind (mem_fun(*this, &AudioClock::field_motion_notify_event), AudioFrames));
	bars_ebox.signal_motion_notify_event().connect (bind (mem_fun(*this, &AudioClock::field_motion_notify_event), Bars));
	beats_ebox.signal_motion_notify_event().connect (bind (mem_fun(*this, &AudioClock::field_motion_notify_event), Beats));
	ticks_ebox.signal_motion_notify_event().connect (bind (mem_fun(*this, &AudioClock::field_motion_notify_event), Ticks));
	ms_hours_ebox.signal_motion_notify_event().connect (bind (mem_fun(*this, &AudioClock::field_motion_notify_event), MS_Hours));
	ms_minutes_ebox.signal_motion_notify_event().connect (bind (mem_fun(*this, &AudioClock::field_motion_notify_event), MS_Minutes));
	ms_seconds_ebox.signal_motion_notify_event().connect (bind (mem_fun(*this, &AudioClock::field_motion_notify_event), MS_Seconds));

	hours_ebox.signal_button_press_event().connect (bind (mem_fun(*this, &AudioClock::field_button_press_event), SMPTE_Hours));
	minutes_ebox.signal_button_press_event().connect (bind (mem_fun(*this, &AudioClock::field_button_press_event), SMPTE_Minutes));
	seconds_ebox.signal_button_press_event().connect (bind (mem_fun(*this, &AudioClock::field_button_press_event), SMPTE_Seconds));
	frames_ebox.signal_button_press_event().connect (bind (mem_fun(*this, &AudioClock::field_button_press_event), SMPTE_Frames));
	audio_frames_ebox.signal_button_press_event().connect (bind (mem_fun(*this, &AudioClock::field_button_press_event), AudioFrames));
	bars_ebox.signal_button_press_event().connect (bind (mem_fun(*this, &AudioClock::field_button_press_event), Bars));
	beats_ebox.signal_button_press_event().connect (bind (mem_fun(*this, &AudioClock::field_button_press_event), Beats));
	ticks_ebox.signal_button_press_event().connect (bind (mem_fun(*this, &AudioClock::field_button_press_event), Ticks));
	ms_hours_ebox.signal_button_press_event().connect (bind (mem_fun(*this, &AudioClock::field_button_press_event), MS_Hours));
	ms_minutes_ebox.signal_button_press_event().connect (bind (mem_fun(*this, &AudioClock::field_button_press_event), MS_Minutes));
	ms_seconds_ebox.signal_button_press_event().connect (bind (mem_fun(*this, &AudioClock::field_button_press_event), MS_Seconds));

	hours_ebox.signal_button_release_event().connect (bind (mem_fun(*this, &AudioClock::field_button_release_event), SMPTE_Hours));
	minutes_ebox.signal_button_release_event().connect (bind (mem_fun(*this, &AudioClock::field_button_release_event), SMPTE_Minutes));
	seconds_ebox.signal_button_release_event().connect (bind (mem_fun(*this, &AudioClock::field_button_release_event), SMPTE_Seconds));
	frames_ebox.signal_button_release_event().connect (bind (mem_fun(*this, &AudioClock::field_button_release_event), SMPTE_Frames));
	audio_frames_ebox.signal_button_release_event().connect (bind (mem_fun(*this, &AudioClock::field_button_release_event), AudioFrames));
	bars_ebox.signal_button_release_event().connect (bind (mem_fun(*this, &AudioClock::field_button_release_event), Bars));
	beats_ebox.signal_button_release_event().connect (bind (mem_fun(*this, &AudioClock::field_button_release_event), Beats));
	ticks_ebox.signal_button_release_event().connect (bind (mem_fun(*this, &AudioClock::field_button_release_event), Ticks));
	ms_hours_ebox.signal_button_release_event().connect (bind (mem_fun(*this, &AudioClock::field_button_release_event), MS_Hours));
	ms_minutes_ebox.signal_button_release_event().connect (bind (mem_fun(*this, &AudioClock::field_button_release_event), MS_Minutes));
	ms_seconds_ebox.signal_button_release_event().connect (bind (mem_fun(*this, &AudioClock::field_button_release_event), MS_Seconds));

	hours_ebox.signal_scroll_event().connect (bind (mem_fun(*this, &AudioClock::field_button_scroll_event), SMPTE_Hours));
	minutes_ebox.signal_scroll_event().connect (bind (mem_fun(*this, &AudioClock::field_button_scroll_event), SMPTE_Minutes));
	seconds_ebox.signal_scroll_event().connect (bind (mem_fun(*this, &AudioClock::field_button_scroll_event), SMPTE_Seconds));
	frames_ebox.signal_scroll_event().connect (bind (mem_fun(*this, &AudioClock::field_button_scroll_event), SMPTE_Frames));
	audio_frames_ebox.signal_scroll_event().connect (bind (mem_fun(*this, &AudioClock::field_button_scroll_event), AudioFrames));
	bars_ebox.signal_scroll_event().connect (bind (mem_fun(*this, &AudioClock::field_button_scroll_event), Bars));
	beats_ebox.signal_scroll_event().connect (bind (mem_fun(*this, &AudioClock::field_button_scroll_event), Beats));
	ticks_ebox.signal_scroll_event().connect (bind (mem_fun(*this, &AudioClock::field_button_scroll_event), Ticks));
	ms_hours_ebox.signal_scroll_event().connect (bind (mem_fun(*this, &AudioClock::field_button_scroll_event), MS_Hours));
	ms_minutes_ebox.signal_scroll_event().connect (bind (mem_fun(*this, &AudioClock::field_button_scroll_event), MS_Minutes));
	ms_seconds_ebox.signal_scroll_event().connect (bind (mem_fun(*this, &AudioClock::field_button_scroll_event), MS_Seconds));

	hours_ebox.signal_key_press_event().connect (bind (mem_fun(*this, &AudioClock::field_key_press_event), SMPTE_Hours));
	minutes_ebox.signal_key_press_event().connect (bind (mem_fun(*this, &AudioClock::field_key_press_event), SMPTE_Minutes));
	seconds_ebox.signal_key_press_event().connect (bind (mem_fun(*this, &AudioClock::field_key_press_event), SMPTE_Seconds));
	frames_ebox.signal_key_press_event().connect (bind (mem_fun(*this, &AudioClock::field_key_press_event), SMPTE_Frames));
	audio_frames_ebox.signal_key_press_event().connect (bind (mem_fun(*this, &AudioClock::field_key_press_event), AudioFrames));
	bars_ebox.signal_key_press_event().connect (bind (mem_fun(*this, &AudioClock::field_key_press_event), Bars));
	beats_ebox.signal_key_press_event().connect (bind (mem_fun(*this, &AudioClock::field_key_press_event), Beats));
	ticks_ebox.signal_key_press_event().connect (bind (mem_fun(*this, &AudioClock::field_key_press_event), Ticks));
	ms_hours_ebox.signal_key_press_event().connect (bind (mem_fun(*this, &AudioClock::field_key_press_event), MS_Hours));
	ms_minutes_ebox.signal_key_press_event().connect (bind (mem_fun(*this, &AudioClock::field_key_press_event), MS_Minutes));
	ms_seconds_ebox.signal_key_press_event().connect (bind (mem_fun(*this, &AudioClock::field_key_press_event), MS_Seconds));

	hours_ebox.signal_key_release_event().connect (bind (mem_fun(*this, &AudioClock::field_key_release_event), SMPTE_Hours));
	minutes_ebox.signal_key_release_event().connect (bind (mem_fun(*this, &AudioClock::field_key_release_event), SMPTE_Minutes));
	seconds_ebox.signal_key_release_event().connect (bind (mem_fun(*this, &AudioClock::field_key_release_event), SMPTE_Seconds));
	frames_ebox.signal_key_release_event().connect (bind (mem_fun(*this, &AudioClock::field_key_release_event), SMPTE_Frames));
	audio_frames_ebox.signal_key_release_event().connect (bind (mem_fun(*this, &AudioClock::field_key_release_event), AudioFrames));
	bars_ebox.signal_key_release_event().connect (bind (mem_fun(*this, &AudioClock::field_key_release_event), Bars));
	beats_ebox.signal_key_release_event().connect (bind (mem_fun(*this, &AudioClock::field_key_release_event), Beats));
	ticks_ebox.signal_key_release_event().connect (bind (mem_fun(*this, &AudioClock::field_key_release_event), Ticks));
	ms_hours_ebox.signal_key_release_event().connect (bind (mem_fun(*this, &AudioClock::field_key_release_event), MS_Hours));
	ms_minutes_ebox.signal_key_release_event().connect (bind (mem_fun(*this, &AudioClock::field_key_release_event), MS_Minutes));
	ms_seconds_ebox.signal_key_release_event().connect (bind (mem_fun(*this, &AudioClock::field_key_release_event), MS_Seconds));

	hours_ebox.signal_focus_in_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_in_event), SMPTE_Hours));
	minutes_ebox.signal_focus_in_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_in_event), SMPTE_Minutes));
	seconds_ebox.signal_focus_in_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_in_event), SMPTE_Seconds));
	frames_ebox.signal_focus_in_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_in_event), SMPTE_Frames));
	audio_frames_ebox.signal_focus_in_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_in_event), AudioFrames));
	bars_ebox.signal_focus_in_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_in_event), Bars));
	beats_ebox.signal_focus_in_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_in_event), Beats));
	ticks_ebox.signal_focus_in_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_in_event), Ticks));
	ms_hours_ebox.signal_focus_in_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_in_event), MS_Hours));
	ms_minutes_ebox.signal_focus_in_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_in_event), MS_Minutes));
	ms_seconds_ebox.signal_focus_in_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_in_event), MS_Seconds));

	hours_ebox.signal_focus_out_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_out_event), SMPTE_Hours));
	minutes_ebox.signal_focus_out_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_out_event), SMPTE_Minutes));
	seconds_ebox.signal_focus_out_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_out_event), SMPTE_Seconds));
	frames_ebox.signal_focus_out_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_out_event), SMPTE_Frames));
	audio_frames_ebox.signal_focus_out_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_out_event), AudioFrames));
	bars_ebox.signal_focus_out_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_out_event), Bars));
	beats_ebox.signal_focus_out_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_out_event), Beats));
	ticks_ebox.signal_focus_out_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_out_event), Ticks));
	ms_hours_ebox.signal_focus_out_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_out_event), MS_Hours));
	ms_minutes_ebox.signal_focus_out_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_out_event), MS_Minutes));
	ms_seconds_ebox.signal_focus_out_event().connect (bind (mem_fun(*this, &AudioClock::field_focus_out_event), MS_Seconds));

	clock_base.signal_focus_in_event().connect (mem_fun (*this, &AudioClock::drop_focus_handler));
}

bool
AudioClock::drop_focus_handler (GdkEventFocus* ignored)
{
	Keyboard::magic_widget_drop_focus ();
	return false;
}

void
AudioClock::on_realize ()
{
	HBox::on_realize ();

	/* styles are not available until the widgets are bound to a window */

	set_size_requests ();
}

void
AudioClock::set (nframes_t when, bool force, nframes_t offset, int which)
{

 	if ((!force && !is_visible()) || session == 0) {
		return;
	}
	
	if (when == last_when && !offset && !force) {
		return;
	}

	bool pdelta = Config->get_primary_clock_delta_edit_cursor();
	bool sdelta = Config->get_secondary_clock_delta_edit_cursor();

	if (offset && which == 1 && pdelta) {
		when = (when > offset) ? when - offset : offset - when;
	} else if (offset && which == 2 && sdelta) {
		when = (when > offset) ? when - offset : offset - when;
	}

	if (which == 1 && pdelta && !last_pdelta) {
		set_widget_name("TransportClockDisplayDelta");
		last_pdelta = true;
	} else if (which == 1 && !pdelta && last_pdelta) {
		set_widget_name("TransportClockDisplay");
		last_pdelta = false;
	} else if (which == 2  && sdelta && !last_sdelta) {
		set_widget_name("SecondaryClockDisplayDelta");
		last_sdelta = true;
	} else if (which == 2 && !sdelta && last_sdelta) {
		set_widget_name("SecondaryClockDisplay");
		last_sdelta = false;
	}

	switch (_mode) {
	case SMPTE:
		set_smpte (when, force);
		break;

	case BBT:
		set_bbt (when, force);
		break;

	case MinSec:
		set_minsec (when, force);
		break;

	case Frames:
		set_frames (when, force);
		break;

	case Off:
		break;
	}

	last_when = when;
}

void
AudioClock::smpte_offset_changed ()
{
	nframes_t current;

	switch (_mode) {
	case SMPTE:
		if (is_duration) {
			current = current_duration();
		} else {
			current = current_time ();
		}
		set (current, true);
		break;
	default:
		break;
	}
}

void
AudioClock::set_frames (nframes_t when, bool force)
{
	char buf[32];
	snprintf (buf, sizeof (buf), "%u", when);
	audio_frames_label.set_text (buf);
	
	if (frames_upper_info_label) {
		nframes_t rate = session->frame_rate();
		
		if (fmod (rate, 1000.0) == 0.000) {
			sprintf (buf, "%uK", rate/1000);
		} else {
			sprintf (buf, "%.3fK", rate/1000.0f);
		}
		
		if (frames_upper_info_label->get_text() != buf) {
			frames_upper_info_label->set_text (buf);
		}
		
		float vid_pullup = Config->get_video_pullup();
		
		if (vid_pullup == 0.0) {
			if (frames_lower_info_label->get_text () != _("none")) {
				frames_lower_info_label->set_text(_("none"));
			}
		} else {
			sprintf (buf, "%-6.4f", vid_pullup);
			if (frames_lower_info_label->get_text() != buf) {
				frames_lower_info_label->set_text (buf);
			}
		}
	}
}	

void
AudioClock::set_minsec (nframes_t when, bool force)
{
	char buf[32];
	nframes_t left;
	int hrs;
	int mins;
	float secs;
	
	left = when;
	hrs = (int) floor (left / (session->frame_rate() * 60.0f * 60.0f));
	left -= (nframes_t) floor (hrs * session->frame_rate() * 60.0f * 60.0f);
	mins = (int) floor (left / (session->frame_rate() * 60.0f));
	left -= (nframes_t) floor (mins * session->frame_rate() * 60.0f);
	secs = left / (float) session->frame_rate();

	if (force || hrs != ms_last_hrs) {
		sprintf (buf, "%02d", hrs);
		ms_hours_label.set_text (buf);
		ms_last_hrs = hrs;
	}

	if (force || mins != ms_last_mins) {
		sprintf (buf, "%02d", mins);
		ms_minutes_label.set_text (buf);
		ms_last_mins = mins;
	}

	if (force || secs != ms_last_secs) {
		sprintf (buf, "%06.3f", secs);
		ms_seconds_label.set_text (buf);
		ms_last_secs = secs;
	}
}

void
AudioClock::set_smpte (nframes_t when, bool force)
{
	char buf[32];
	SMPTE::Time smpte;
	
	if (is_duration) {
		session->smpte_duration (when, smpte);
	} else {
		session->smpte_time (when, smpte);
	}

	if (force || smpte.hours != last_hrs || smpte.negative != last_negative) {
		if (smpte.negative) {
			sprintf (buf, "-%02" PRIu32, smpte.hours);
		} else {
			sprintf (buf, " %02" PRIu32, smpte.hours);
		}
		hours_label.set_text (buf);
		last_hrs = smpte.hours;
		last_negative = smpte.negative;
	}

	if (force || smpte.minutes != last_mins) {
		sprintf (buf, "%02" PRIu32, smpte.minutes);
		minutes_label.set_text (buf);
		last_mins = smpte.minutes;
	}

	if (force || smpte.seconds != last_secs) {
		sprintf (buf, "%02" PRIu32, smpte.seconds);
		seconds_label.set_text (buf);
		last_secs = smpte.seconds;
	}

	if (force || smpte.frames != last_frames) {
		sprintf (buf, "%02" PRIu32, smpte.frames);
		frames_label.set_text (buf);
		last_frames = smpte.frames;
	}
	
	if (smpte_upper_info_label) {
		double smpte_frames = session->smpte_frames_per_second();
		
		if ( fmod(smpte_frames, 1.0) == 0.0) {
			sprintf (buf, "%u", int (smpte_frames)); 
		} else {
			sprintf (buf, "%.2f", smpte_frames);
		}
		
		if (smpte_upper_info_label->get_text() != buf) {
			smpte_upper_info_label->set_text (buf);
		}
		
		if ((fabs(smpte_frames - 29.97) < 0.0001) || smpte_frames == 30) {
			if (session->smpte_drop_frames()) {
				sprintf (buf, "DF");
			} else {
				sprintf (buf, "NDF");
			}
		} else {
			// there is no drop frame alternative
			buf[0] = '\0';
		}
		
		if (smpte_lower_info_label->get_text() != buf) {
			smpte_lower_info_label->set_text (buf);
		}
	}
}

void
AudioClock::set_bbt (nframes_t when, bool force)
{
	char buf[16];
	BBT_Time bbt;

	/* handle a common case */
	if (is_duration) {
		if (when == 0) {
			bbt.bars = 0;
			bbt.beats = 0;
			bbt.ticks = 0;	
		} else {
			session->tempo_map().bbt_time (when, bbt);
			bbt.bars--;
			bbt.beats--;
		}
	} else {
		session->tempo_map().bbt_time (when, bbt);
	}

	sprintf (buf, "%03" PRIu32, bbt.bars);
	if (force || bars_label.get_text () != buf) {
		bars_label.set_text (buf);
	}
	sprintf (buf, "%02" PRIu32, bbt.beats);
	if (force || beats_label.get_text () != buf) {
		beats_label.set_text (buf);
	}
	sprintf (buf, "%04" PRIu32, bbt.ticks);
	if (force || ticks_label.get_text () != buf) {
		ticks_label.set_text (buf);
	}
	
	if (bbt_upper_info_label) {
		nframes64_t pos;

		if (bbt_reference_time < 0) {
			pos = when;
		} else {
			pos = bbt_reference_time;
		}

		TempoMap::Metric m (session->tempo_map().metric_at (pos));

		sprintf (buf, "%-5.2f", m.tempo().beats_per_minute());
		if (bbt_lower_info_label->get_text() != buf) {
			bbt_lower_info_label->set_text (buf);
		}
		sprintf (buf, "%g|%g", m.meter().beats_per_bar(), m.meter().note_divisor());
		if (bbt_upper_info_label->get_text() != buf) {
			bbt_upper_info_label->set_text (buf);
		}
	}
}

void
AudioClock::set_session (Session *s)
{
	session = s;

	if (s) {

		XMLProperty* prop;
		XMLNode* node = session->extra_xml (X_("ClockModes"));
		AudioClock::Mode amode;
		
		if (node) {
			if ((prop = node->property (_name)) != 0) {
				amode = AudioClock::Mode (string_2_enum (prop->value(), amode));
				set_mode (amode);
			}
		}

		set (last_when, true);
	}
}

void
AudioClock::focus ()
{
	switch (_mode) {
	case SMPTE:
		hours_ebox.grab_focus ();
		break;

	case BBT:
		bars_ebox.grab_focus ();
		break;

	case MinSec:
		ms_hours_ebox.grab_focus ();
		break;

	case Frames:
		frames_ebox.grab_focus ();
		break;

	case Off:
		break;
	}
}


bool
AudioClock::field_key_press_event (GdkEventKey *ev, Field field)
{
	/* all key activity is handled on key release */
	return true;
}

bool
AudioClock::field_key_release_event (GdkEventKey *ev, Field field)
{
	Label *label = 0;
	string new_text;
	char new_char = 0;
	bool move_on = false;

	switch (field) {
	case SMPTE_Hours:
		label = &hours_label;
		break;
	case SMPTE_Minutes:
		label = &minutes_label;
		break;
	case SMPTE_Seconds:
		label = &seconds_label;
		break;
	case SMPTE_Frames:
		label = &frames_label;
		break;

	case AudioFrames:
		label = &audio_frames_label;
		break;

	case MS_Hours:
		label = &ms_hours_label;
		break;
	case MS_Minutes:
		label = &ms_minutes_label;
		break;
	case MS_Seconds:
		label = &ms_seconds_label;
		break;

	case Bars:
		label = &bars_label;
		break;
	case Beats:
		label = &beats_label;
		break;
	case Ticks:
		label = &ticks_label;
		break;
	default:
		return false;
	}

	switch (ev->keyval) {
	case GDK_0:
	case GDK_KP_0:
		new_char = '0';
		break;
	case GDK_1:
	case GDK_KP_1:
		new_char = '1';
		break;
	case GDK_2:
	case GDK_KP_2:
		new_char = '2';
		break;
	case GDK_3:
	case GDK_KP_3:
		new_char = '3';
		break;
	case GDK_4:
	case GDK_KP_4:
		new_char = '4';
		break;
	case GDK_5:
	case GDK_KP_5:
		new_char = '5';
		break;
	case GDK_6:
	case GDK_KP_6:
		new_char = '6';
		break;
	case GDK_7:
	case GDK_KP_7:
		new_char = '7';
		break;
	case GDK_8:
	case GDK_KP_8:
		new_char = '8';
		break;
	case GDK_9:
	case GDK_KP_9:
		new_char = '9';
		break;

	case GDK_period:
	case GDK_KP_Decimal:
		if (_mode == MinSec && field == MS_Seconds) {
			new_char = '.';
		} else {
			return false;
		}
		break;

	case GDK_Tab:
	case GDK_Return:
	case GDK_KP_Enter:
		move_on = true;
		break;

	case GDK_Escape:
		key_entry_state = 0;
		clock_base.grab_focus ();
		ChangeAborted();  /*  EMIT SIGNAL  */
		return true;

	default:
		return false;
	}

	if (!move_on) {

		if (key_entry_state == 0) {

			/* initialize with a fresh new string */

			if (field != AudioFrames) {
				for (uint32_t xn = 0; xn < field_length[field] - 1; ++xn) {
					new_text += '0';
				}
			} else {
				new_text = "";
			}

		} else {

			string existing = label->get_text();
			if (existing.length() >= field_length[field]) {
				new_text = existing.substr (1, field_length[field] - 1);
			} else {
				new_text = existing.substr (0, field_length[field] - 1);
			}
		}

		new_text += new_char;
		label->set_text (new_text);
		key_entry_state++;
	}

	if (key_entry_state == field_length[field]) {
		move_on = true;
	}
	
	if (move_on) {

		if (key_entry_state) {
      
			switch (field) {
			case SMPTE_Hours:
			case SMPTE_Minutes:
			case SMPTE_Seconds:
			case SMPTE_Frames:
				// Check SMPTE fields for sanity (may also adjust fields)
				smpte_sanitize_display();
				break;
			case Bars:
			case Beats:
			case Ticks:
				// Bars should never be, unless this clock is for a duration
				if (atoi(bars_label.get_text()) == 0 && !is_duration) {
					bars_label.set_text("001");
				}
				//  beats should never be 0, unless this clock is for a duration
				if (atoi(beats_label.get_text()) == 0 && !is_duration) {
					beats_label.set_text("01");
				}
				break;
			default:
				break;
			}
			
			ValueChanged(); /* EMIT_SIGNAL */
		}
		
		/* move on to the next field.
		 */
		
		switch (field) {
			
			/* SMPTE */
			
		case SMPTE_Hours:
			minutes_ebox.grab_focus ();
			break;
		case SMPTE_Minutes:
			seconds_ebox.grab_focus ();
			break;
		case SMPTE_Seconds:
			frames_ebox.grab_focus ();
			break;
		case SMPTE_Frames:
			clock_base.grab_focus ();
			break;

		/* audio frames */
		case AudioFrames:
			clock_base.grab_focus ();
			break;

		/* Min:Sec */

		case MS_Hours:
			ms_minutes_ebox.grab_focus ();
			break;
		case MS_Minutes:
			ms_seconds_ebox.grab_focus ();
			break;
		case MS_Seconds:
			clock_base.grab_focus ();
			break;

		/* BBT */

		case Bars:
			beats_ebox.grab_focus ();
			break;
		case Beats:
			ticks_ebox.grab_focus ();
			break;
		case Ticks:
			clock_base.grab_focus ();
			break;

		default:
			break;
		}

	}

	//if user hit Enter, lose focus
	switch (ev->keyval) {
	case GDK_Return:
	case GDK_KP_Enter:
		clock_base.grab_focus ();
	}

	return true;
}

bool
AudioClock::field_focus_in_event (GdkEventFocus *ev, Field field)
{
	key_entry_state = 0;

	Keyboard::magic_widget_grab_focus ();

	switch (field) {
	case SMPTE_Hours:
		hours_ebox.set_flags (Gtk::HAS_FOCUS);
		hours_ebox.set_state (Gtk::STATE_ACTIVE);
		break;
	case SMPTE_Minutes:
		minutes_ebox.set_flags (Gtk::HAS_FOCUS);
		minutes_ebox.set_state (Gtk::STATE_ACTIVE);
		break;
	case SMPTE_Seconds:
		seconds_ebox.set_flags (Gtk::HAS_FOCUS);
		seconds_ebox.set_state (Gtk::STATE_ACTIVE);
		break;
	case SMPTE_Frames:
		frames_ebox.set_flags (Gtk::HAS_FOCUS);
		frames_ebox.set_state (Gtk::STATE_ACTIVE);
		break;

	case AudioFrames:
		audio_frames_ebox.set_flags (Gtk::HAS_FOCUS);
		audio_frames_ebox.set_state (Gtk::STATE_ACTIVE);
		break;

	case MS_Hours:
		ms_hours_ebox.set_flags (Gtk::HAS_FOCUS);
		ms_hours_ebox.set_state (Gtk::STATE_ACTIVE);
		break;
	case MS_Minutes:
		ms_minutes_ebox.set_flags (Gtk::HAS_FOCUS);
		ms_minutes_ebox.set_state (Gtk::STATE_ACTIVE);
		break;
	case MS_Seconds:
		ms_seconds_ebox.set_flags (Gtk::HAS_FOCUS);
		ms_seconds_ebox.set_state (Gtk::STATE_ACTIVE);
		break;
	case Bars:
		bars_ebox.set_flags (Gtk::HAS_FOCUS);
		bars_ebox.set_state (Gtk::STATE_ACTIVE);
		break;
	case Beats:
		beats_ebox.set_flags (Gtk::HAS_FOCUS);
		beats_ebox.set_state (Gtk::STATE_ACTIVE);
		break;
	case Ticks:
		ticks_ebox.set_flags (Gtk::HAS_FOCUS);
		ticks_ebox.set_state (Gtk::STATE_ACTIVE);
		break;
	}

	return false;
}

bool
AudioClock::field_focus_out_event (GdkEventFocus *ev, Field field)
{
	switch (field) {

	case SMPTE_Hours:
		hours_ebox.unset_flags (Gtk::HAS_FOCUS);
		hours_ebox.set_state (Gtk::STATE_NORMAL);
		break;
	case SMPTE_Minutes:
		minutes_ebox.unset_flags (Gtk::HAS_FOCUS);
		minutes_ebox.set_state (Gtk::STATE_NORMAL);
		break;
	case SMPTE_Seconds:
		seconds_ebox.unset_flags (Gtk::HAS_FOCUS);
		seconds_ebox.set_state (Gtk::STATE_NORMAL);
		break;
	case SMPTE_Frames:
		frames_ebox.unset_flags (Gtk::HAS_FOCUS);
		frames_ebox.set_state (Gtk::STATE_NORMAL);
		break;

	case AudioFrames:
		audio_frames_ebox.unset_flags (Gtk::HAS_FOCUS);
		audio_frames_ebox.set_state (Gtk::STATE_NORMAL);
		break;

	case MS_Hours:
		ms_hours_ebox.unset_flags (Gtk::HAS_FOCUS);
		ms_hours_ebox.set_state (Gtk::STATE_NORMAL);
		break;
	case MS_Minutes:
		ms_minutes_ebox.unset_flags (Gtk::HAS_FOCUS);
		ms_minutes_ebox.set_state (Gtk::STATE_NORMAL);
		break;
	case MS_Seconds:
		ms_seconds_ebox.unset_flags (Gtk::HAS_FOCUS);
		ms_seconds_ebox.set_state (Gtk::STATE_NORMAL);
		break;

	case Bars:
		bars_ebox.unset_flags (Gtk::HAS_FOCUS);
		bars_ebox.set_state (Gtk::STATE_NORMAL);
		break;
	case Beats:
		beats_ebox.unset_flags (Gtk::HAS_FOCUS);
		beats_ebox.set_state (Gtk::STATE_NORMAL);
		break;
	case Ticks:
		ticks_ebox.unset_flags (Gtk::HAS_FOCUS);
		ticks_ebox.set_state (Gtk::STATE_NORMAL);
		break;
	}

	Keyboard::magic_widget_drop_focus ();

	return false;
}

bool
AudioClock::field_button_release_event (GdkEventButton *ev, Field field)
{
	if (dragging) {
		gdk_pointer_ungrab (GDK_CURRENT_TIME);
		dragging = false;
		if (ev->y > drag_start_y+1 || ev->y < drag_start_y-1 || Keyboard::modifier_state_equals (ev->state, Keyboard::TertiaryModifier)){
			// we actually dragged so return without setting editing focus, or we shift clicked
			return true;
		}
	}

	if (!editable) {
		if (ops_menu == 0) {
			build_ops_menu ();
		}
		ops_menu->popup (1, ev->time);
		return true;
	}

	if (Keyboard::is_context_menu_event (ev)) {
		if (ops_menu == 0) {
			build_ops_menu ();
		}
		ops_menu->popup (1, ev->time);
		return true;
	} 

	switch (ev->button) {
	case 1:
		switch (field) {
		case SMPTE_Hours:
			hours_ebox.grab_focus();
			break;
		case SMPTE_Minutes:
			minutes_ebox.grab_focus();
			break;
		case SMPTE_Seconds:
			seconds_ebox.grab_focus();
			break;
		case SMPTE_Frames:
			frames_ebox.grab_focus();
			break;

		case AudioFrames:
			audio_frames_ebox.grab_focus();
			break;
			
		case MS_Hours:
			ms_hours_ebox.grab_focus();
			break;
		case MS_Minutes:
			ms_minutes_ebox.grab_focus();
			break;
		case MS_Seconds:
			ms_seconds_ebox.grab_focus();
			break;
			
		case Bars:
			bars_ebox.grab_focus ();
			break;
		case Beats:
			beats_ebox.grab_focus ();
			break;
		case Ticks:
			ticks_ebox.grab_focus ();
			break;
		}
		break;
		
	default:
		break;
	}

	return true;
}

bool
AudioClock::field_button_press_event (GdkEventButton *ev, Field field)
{
	if (session == 0) return false;

	nframes_t frames = 0;

	switch (ev->button) {
	case 1:
		if (Keyboard::modifier_state_equals (ev->state, Keyboard::TertiaryModifier)) {
			set (frames, true);
			ValueChanged (); /* EMIT_SIGNAL */
					}
	
                /* make absolutely sure that the pointer is grabbed */
		gdk_pointer_grab(ev->window,false ,
				 GdkEventMask( Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_PRESS_MASK |Gdk::BUTTON_RELEASE_MASK), 
				 NULL,NULL,ev->time);
		dragging = true;
		drag_accum = 0;
		drag_start_y = ev->y;
		drag_y = ev->y;
		break;

	case 2:
		if (Keyboard::modifier_state_equals (ev->state, Keyboard::TertiaryModifier)) {
			set (frames, true);
			ValueChanged (); /* EMIT_SIGNAL */
		}
		break;

	case 3:
		/* used for context sensitive menu */
		return false;
		break;

	default:
		return false;
		break;
	}
	
	return true;
}

bool
AudioClock::field_button_scroll_event (GdkEventScroll *ev, Field field)
{
	if (session == 0) {
		return false;
	}

	nframes_t frames = 0;

	switch (ev->direction) {

	case GDK_SCROLL_UP:
	       frames = get_frames (field);
	       if (frames != 0) {
		      if (Keyboard::modifier_state_equals (ev->state, Keyboard::PrimaryModifier)) {
			     frames *= 10;
		      }
		      set (current_time() + frames, true);
		      ValueChanged (); /* EMIT_SIGNAL */
	       }
	       break;

	case GDK_SCROLL_DOWN:
	       frames = get_frames (field);
	       if (frames != 0) {
		      if (Keyboard::modifier_state_equals (ev->state, Keyboard::PrimaryModifier)) {
			     frames *= 10;
		      }
		      
		      if ((double)current_time() - (double)frames < 0.0) {
			     set (0, true);
		      } else {
			     set (current_time() - frames, true);
		      }
		      
		      ValueChanged (); /* EMIT_SIGNAL */
	       }
	       break;

	default:
		return false;
		break;
	}
	
	return true;
}

bool
AudioClock::field_motion_notify_event (GdkEventMotion *ev, Field field)
{
	if (session == 0 || !dragging) {
		return false;
	}
	
	float pixel_frame_scale_factor = 0.2f;

/*
	if (Keyboard::modifier_state_equals (ev->state, Keyboard::PrimaryModifier))  {
		pixel_frame_scale_factor = 0.1f;
	}


	if (Keyboard::modifier_state_contains (ev->state, 
					       Keyboard::PrimaryModifier|Keyboard::SecondaryModifier)) {

		pixel_frame_scale_factor = 0.025f;
	}
*/
	double y_delta = ev->y - drag_y;

	drag_accum +=  y_delta*pixel_frame_scale_factor;

	drag_y = ev->y;

	if (trunc(drag_accum) != 0) {

		nframes_t frames;
		nframes_t pos ;
		int dir;
		dir = (drag_accum < 0 ? 1:-1);
		pos = current_time();
		frames = get_frames (field,pos,dir);
		
		if (frames  != 0 &&  frames * drag_accum < current_time()) {
		
			set ((nframes_t) floor (pos - drag_accum * frames), false); // minus because up is negative in computer-land
		
		} else {
			set (0 , false);
		
 		}

	       	drag_accum= 0;
		ValueChanged();	 /* EMIT_SIGNAL */	
		

	}

	return true;
}

nframes_t
AudioClock::get_frames (Field field,nframes_t pos,int dir)
{

	nframes_t frames = 0;
	BBT_Time bbt;
	switch (field) {
	case SMPTE_Hours:
		frames = (nframes_t) floor (3600.0 * session->frame_rate());
		break;
	case SMPTE_Minutes:
		frames = (nframes_t) floor (60.0 * session->frame_rate());
		break;
	case SMPTE_Seconds:
		frames = session->frame_rate();
		break;
	case SMPTE_Frames:
		frames = (nframes_t) floor (session->frame_rate() / session->smpte_frames_per_second());
		break;

	case AudioFrames:
		frames = 1;
		break;

	case MS_Hours:
		frames = (nframes_t) floor (3600.0 * session->frame_rate());
		break;
	case MS_Minutes:
		frames = (nframes_t) floor (60.0 * session->frame_rate());
		break;
	case MS_Seconds:
		frames = session->frame_rate();
		break;

	case Bars:
		bbt.bars = 1;
		bbt.beats = 0;
		bbt.ticks = 0;
		frames = session->tempo_map().bbt_duration_at(pos,bbt,dir);
		break;
	case Beats:
		bbt.bars = 0;
		bbt.beats = 1;
		bbt.ticks = 0;
		frames = session->tempo_map().bbt_duration_at(pos,bbt,dir);
		break;
	case Ticks:
		bbt.bars = 0;
		bbt.beats = 0;
		bbt.ticks = 1;
		frames = session->tempo_map().bbt_duration_at(pos,bbt,dir);
		break;
	}

	return frames;
}

nframes_t
AudioClock::current_time (nframes_t pos) const
{
	nframes_t ret = 0;

	switch (_mode) {
	case SMPTE:
		ret = smpte_frame_from_display ();
		break;
	case BBT:
		ret = bbt_frame_from_display (pos);
		break;

	case MinSec:
		ret = minsec_frame_from_display ();
		break;

	case Frames:
		ret = audio_frame_from_display ();
		break;

	case Off:
		break;
	}

	return ret;
}

nframes_t
AudioClock::current_duration (nframes_t pos) const
{
	nframes_t ret = 0;

	switch (_mode) {
	case SMPTE:
		ret = smpte_frame_from_display ();
		break;
	case BBT:
		ret = bbt_frame_duration_from_display (pos);
		break;

	case MinSec:
		ret = minsec_frame_from_display ();
		break;

	case Frames:
		ret = audio_frame_from_display ();
		break;

	case Off:
		break;
	}

	return ret;
}

void
AudioClock::smpte_sanitize_display()
{
	// Check SMPTE fields for sanity, possibly adjusting values
	if (atoi(minutes_label.get_text()) > 59) {
		minutes_label.set_text("59");
	}
	
	if (atoi(seconds_label.get_text()) > 59) {
		seconds_label.set_text("59");
	}
	
	switch ((long)rint(session->smpte_frames_per_second())) {
	case 24:
		if (atoi(frames_label.get_text()) > 23) {
			frames_label.set_text("23");
		}
		break;
	case 25:
		if (atoi(frames_label.get_text()) > 24) {
			frames_label.set_text("24");
		}
		break;
	case 30:
		if (atoi(frames_label.get_text()) > 29) {
			frames_label.set_text("29");
		}
		break;
	default:
		break;
	}
	
	if (session->smpte_drop_frames()) {
		if ((atoi(minutes_label.get_text()) % 10) && (atoi(seconds_label.get_text()) == 0) && (atoi(frames_label.get_text()) < 2)) {
			frames_label.set_text("02");
		}
	}
}

nframes_t
AudioClock::smpte_frame_from_display () const
{
	if (session == 0) {
		return 0;
	}
	
	SMPTE::Time smpte;
	nframes_t sample;
	
	smpte.hours = atoi (hours_label.get_text());
	smpte.minutes = atoi (minutes_label.get_text());
	smpte.seconds = atoi (seconds_label.get_text());
	smpte.frames = atoi (frames_label.get_text());
	smpte.rate = session->smpte_frames_per_second();
	smpte.drop= session->smpte_drop_frames();

	session->smpte_to_sample( smpte, sample, false /* use_offset */, false /* use_subframes */ );
	

#if 0
#define SMPTE_SAMPLE_TEST_1
#define SMPTE_SAMPLE_TEST_2
#define SMPTE_SAMPLE_TEST_3
#define SMPTE_SAMPLE_TEST_4
#define SMPTE_SAMPLE_TEST_5
#define SMPTE_SAMPLE_TEST_6
#define SMPTE_SAMPLE_TEST_7

	// Testcode for smpte<->sample conversions (P.S.)
	SMPTE::Time smpte1;
	nframes_t sample1;
	nframes_t oldsample = 0;
	SMPTE::Time smpte2;
	nframes_t sample_increment;

	sample_increment = (long)rint(session->frame_rate() / session->smpte_frames_per_second);

#ifdef SMPTE_SAMPLE_TEST_1
	// Test 1: use_offset = false, use_subframes = false
	cout << "use_offset = false, use_subframes = false" << endl;
	for (int i = 0; i < 108003; i++) {
		session->smpte_to_sample( smpte1, sample1, false /* use_offset */, false /* use_subframes */ );
		session->sample_to_smpte( sample1, smpte2, false /* use_offset */, false /* use_subframes */ );

		if ((i > 0) && ( ((sample1 - oldsample) != sample_increment) && ((sample1 - oldsample) != (sample_increment + 1)) && ((sample1 - oldsample) != (sample_increment - 1)))) {
			cout << "ERROR: sample increment not right: " << (sample1 - oldsample) << " != " << sample_increment << endl;
			cout << "smpte1: " << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
			cout << "sample: " << sample1 << endl;
			cout << "sample: " << sample1 << " -> ";
			cout << "smpte2: " << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
			break;
		}
    
		if (smpte2.hours != smpte1.hours || smpte2.minutes != smpte1.minutes || smpte2.seconds != smpte2.seconds || smpte2.frames != smpte1.frames) {
			cout << "ERROR: smpte2 not equal smpte1" << endl;
			cout << "smpte1: " << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
			cout << "sample: " << sample1 << endl;
			cout << "sample: " << sample1 << " -> ";
			cout << "smpte2: " << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
			break;
		}
		oldsample = sample1;
		session->smpte_increment( smpte1 );
	}

	cout << "sample_increment: " << sample_increment << endl;
	cout << "sample: " << sample1 << " -> ";
	cout << "smpte: " << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
#endif

#ifdef SMPTE_SAMPLE_TEST_2
	// Test 2: use_offset = true, use_subframes = false
	cout << "use_offset = true, use_subframes = false" << endl;
  
	smpte1.hours = 0;
	smpte1.minutes = 0;
	smpte1.seconds = 0;
	smpte1.frames = 0;
	smpte1.subframes = 0;
	sample1 = oldsample = 0;

	session->sample_to_smpte( sample1, smpte1, true /* use_offset */, false /* use_subframes */ );
	cout << "Starting at sample: " << sample1 << " -> ";
	cout << "smpte: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << endl;

	for (int i = 0; i < 108003; i++) {
		session->smpte_to_sample( smpte1, sample1, true /* use_offset */, false /* use_subframes */ );
		session->sample_to_smpte( sample1, smpte2, true /* use_offset */, false /* use_subframes */ );

//     cout << "smpte: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
//     cout << "sample: " << sample1 << endl;
//     cout << "sample: " << sample1 << " -> ";
//     cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
    
		if ((i > 0) && ( ((sample1 - oldsample) != sample_increment) && ((sample1 - oldsample) != (sample_increment + 1)) && ((sample1 - oldsample) != (sample_increment - 1)))) {
			cout << "ERROR: sample increment not right: " << (sample1 - oldsample) << " != " << sample_increment << endl;
			cout << "smpte1: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
			cout << "sample: " << sample1 << endl;
			cout << "sample: " << sample1 << " -> ";
			cout << "smpte2: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
			break;
		}
    
		if (smpte2.hours != smpte1.hours || smpte2.minutes != smpte1.minutes || smpte2.seconds != smpte2.seconds || smpte2.frames != smpte1.frames) {
			cout << "ERROR: smpte2 not equal smpte1" << endl;
			cout << "smpte1: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
			cout << "sample: " << sample1 << endl;
			cout << "sample: " << sample1 << " -> ";
			cout << "smpte2: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
			break;
		}
		oldsample = sample1;
		session->smpte_increment( smpte1 );
	}

	cout << "sample_increment: " << sample_increment << endl;
	cout << "sample: " << sample1 << " -> ";
	cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
#endif

#ifdef SMPTE_SAMPLE_TEST_3
	// Test 3: use_offset = true, use_subframes = false, decrement
	cout << "use_offset = true, use_subframes = false, decrement" << endl;  

	session->sample_to_smpte( sample1, smpte1, true /* use_offset */, false /* use_subframes */ );
	cout << "Starting at sample: " << sample1 << " -> ";
	cout << "smpte: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << endl;

	for (int i = 0; i < 108003; i++) {
		session->smpte_to_sample( smpte1, sample1, true /* use_offset */, false /* use_subframes */ );
		session->sample_to_smpte( sample1, smpte2, true /* use_offset */, false /* use_subframes */ );

//     cout << "smpte: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
//     cout << "sample: " << sample1 << endl;
//     cout << "sample: " << sample1 << " -> ";
//     cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
    
		if ((i > 0) && ( ((oldsample - sample1) != sample_increment) && ((oldsample - sample1) != (sample_increment + 1)) && ((oldsample - sample1) != (sample_increment - 1)))) {
			cout << "ERROR: sample increment not right: " << (oldsample - sample1) << " != " << sample_increment << endl;
			cout << "smpte1: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
			cout << "sample: " << sample1 << endl;
			cout << "sample: " << sample1 << " -> ";
			cout << "smpte2: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
			break;
		}
    
		if (smpte2.hours != smpte1.hours || smpte2.minutes != smpte1.minutes || smpte2.seconds != smpte2.seconds || smpte2.frames != smpte1.frames) {
			cout << "ERROR: smpte2 not equal smpte1" << endl;
			cout << "smpte1: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
			cout << "sample: " << sample1 << endl;
			cout << "sample: " << sample1 << " -> ";
			cout << "smpte2: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
			break;
		}
		oldsample = sample1;
		session->smpte_decrement( smpte1 );
	}

	cout << "sample_decrement: " << sample_increment << endl;
	cout << "sample: " << sample1 << " -> ";
	cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
#endif


#ifdef SMPTE_SAMPLE_TEST_4
	// Test 4: use_offset = true, use_subframes = true
	cout << "use_offset = true, use_subframes = true" << endl;
  
	for (long sub = 5; sub < 80; sub += 5) {
		smpte1.hours = 0;
		smpte1.minutes = 0;
		smpte1.seconds = 0;
		smpte1.frames = 0;
		smpte1.subframes = 0;
		sample1 = oldsample = (sample_increment * sub) / 80;
    
		session->sample_to_smpte( sample1, smpte1, true /* use_offset */, true /* use_subframes */ );
    
		cout << "starting at sample: " << sample1 << " -> ";
		cout << "smpte: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << endl;
    
		for (int i = 0; i < 108003; i++) {
			session->smpte_to_sample( smpte1, sample1, true /* use_offset */, true /* use_subframes */ );
			session->sample_to_smpte( sample1, smpte2, true /* use_offset */, true /* use_subframes */ );
      
			if ((i > 0) && ( ((sample1 - oldsample) != sample_increment) && ((sample1 - oldsample) != (sample_increment + 1)) && ((sample1 - oldsample) != (sample_increment - 1)))) {
				cout << "ERROR: sample increment not right: " << (sample1 - oldsample) << " != " << sample_increment << endl;
				cout << "smpte1: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
				cout << "sample: " << sample1 << endl;
				cout << "sample: " << sample1 << " -> ";
				cout << "smpte2: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
				//break;
			}
      
			if (smpte2.hours != smpte1.hours || smpte2.minutes != smpte1.minutes || smpte2.seconds != smpte2.seconds || smpte2.frames != smpte1.frames || smpte2.subframes != smpte1.subframes) {
				cout << "ERROR: smpte2 not equal smpte1" << endl;
				cout << "smpte1: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
				cout << "sample: " << sample1 << endl;
				cout << "sample: " << sample1 << " -> ";
				cout << "smpte2: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
				break;
			}
			oldsample = sample1;
			session->smpte_increment( smpte1 );
		}
    
		cout << "sample_increment: " << sample_increment << endl;
		cout << "sample: " << sample1 << " -> ";
		cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;

		for (int i = 0; i < 108003; i++) {
			session->smpte_to_sample( smpte1, sample1, true /* use_offset */, true /* use_subframes */ );
			session->sample_to_smpte( sample1, smpte2, true /* use_offset */, true /* use_subframes */ );
      
			if ((i > 0) && ( ((oldsample - sample1) != sample_increment) && ((oldsample - sample1) != (sample_increment + 1)) && ((oldsample - sample1) != (sample_increment - 1)))) {
				cout << "ERROR: sample increment not right: " << (oldsample - sample1) << " != " << sample_increment << endl;
				cout << "smpte1: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
				cout << "sample: " << sample1 << endl;
				cout << "sample: " << sample1 << " -> ";
				cout << "smpte2: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
				//break;
			}
      
			if (smpte2.hours != smpte1.hours || smpte2.minutes != smpte1.minutes || smpte2.seconds != smpte2.seconds || smpte2.frames != smpte1.frames || smpte2.subframes != smpte1.subframes) {
				cout << "ERROR: smpte2 not equal smpte1" << endl;
				cout << "smpte1: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
				cout << "sample: " << sample1 << endl;
				cout << "sample: " << sample1 << " -> ";
				cout << "smpte2: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
				break;
			}
			oldsample = sample1;
			session->smpte_decrement( smpte1 );
		}
    
		cout << "sample_decrement: " << sample_increment << endl;
		cout << "sample: " << sample1 << " -> ";
		cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
	}
#endif


#ifdef SMPTE_SAMPLE_TEST_5
	// Test 5: use_offset = true, use_subframes = false, increment seconds
	cout << "use_offset = true, use_subframes = false, increment seconds" << endl;
  
	smpte1.hours = 0;
	smpte1.minutes = 0;
	smpte1.seconds = 0;
	smpte1.frames = 0;
	smpte1.subframes = 0;
	sample1 = oldsample = 0;
	sample_increment = session->frame_rate();

	session->sample_to_smpte( sample1, smpte1, true /* use_offset */, false /* use_subframes */ );
	cout << "Starting at sample: " << sample1 << " -> ";
	cout << "smpte: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << endl;

	for (int i = 0; i < 3600; i++) {
		session->smpte_to_sample( smpte1, sample1, true /* use_offset */, false /* use_subframes */ );
		session->sample_to_smpte( sample1, smpte2, true /* use_offset */, false /* use_subframes */ );

//     cout << "smpte: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
//     cout << "sample: " << sample1 << endl;
//     cout << "sample: " << sample1 << " -> ";
//     cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
    
//     if ((i > 0) && ( ((sample1 - oldsample) != sample_increment) && ((sample1 - oldsample) != (sample_increment + 1)) && ((sample1 - oldsample) != (sample_increment - 1))))
//     {
//       cout << "ERROR: sample increment not right: " << (sample1 - oldsample) << " != " << sample_increment << endl;
//       break;
//     }
    
		if (smpte2.hours != smpte1.hours || smpte2.minutes != smpte1.minutes || smpte2.seconds != smpte2.seconds || smpte2.frames != smpte1.frames) {
			cout << "ERROR: smpte2 not equal smpte1" << endl;
			cout << "smpte: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
			cout << "sample: " << sample1 << endl;
			cout << "sample: " << sample1 << " -> ";
			cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
			break;
		}
		oldsample = sample1;
		session->smpte_increment_seconds( smpte1 );
	}

	cout << "sample_increment: " << sample_increment << endl;
	cout << "sample: " << sample1 << " -> ";
	cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
#endif


#ifdef SMPTE_SAMPLE_TEST_6
	// Test 6: use_offset = true, use_subframes = false, increment minutes
	cout << "use_offset = true, use_subframes = false, increment minutes" << endl;
  
	smpte1.hours = 0;
	smpte1.minutes = 0;
	smpte1.seconds = 0;
	smpte1.frames = 0;
	smpte1.subframes = 0;
	sample1 = oldsample = 0;
	sample_increment = session->frame_rate() * 60;

	session->sample_to_smpte( sample1, smpte1, true /* use_offset */, false /* use_subframes */ );
	cout << "Starting at sample: " << sample1 << " -> ";
	cout << "smpte: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << endl;

	for (int i = 0; i < 60; i++) {
		session->smpte_to_sample( smpte1, sample1, true /* use_offset */, false /* use_subframes */ );
		session->sample_to_smpte( sample1, smpte2, true /* use_offset */, false /* use_subframes */ );

//     cout << "smpte: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
//     cout << "sample: " << sample1 << endl;
//     cout << "sample: " << sample1 << " -> ";
//     cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
    
//     if ((i > 0) && ( ((sample1 - oldsample) != sample_increment) && ((sample1 - oldsample) != (sample_increment + 1)) && ((sample1 - oldsample) != (sample_increment - 1))))
//     {
//       cout << "ERROR: sample increment not right: " << (sample1 - oldsample) << " != " << sample_increment << endl;
//       break;
//     }
    
		if (smpte2.hours != smpte1.hours || smpte2.minutes != smpte1.minutes || smpte2.seconds != smpte2.seconds || smpte2.frames != smpte1.frames) {
			cout << "ERROR: smpte2 not equal smpte1" << endl;
			cout << "smpte: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
			cout << "sample: " << sample1 << endl;
			cout << "sample: " << sample1 << " -> ";
			cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
			break;
		}
		oldsample = sample1;
		session->smpte_increment_minutes( smpte1 );
	}

	cout << "sample_increment: " << sample_increment << endl;
	cout << "sample: " << sample1 << " -> ";
	cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
#endif

#ifdef SMPTE_SAMPLE_TEST_7
	// Test 7: use_offset = true, use_subframes = false, increment hours
	cout << "use_offset = true, use_subframes = false, increment hours" << endl;
  
	smpte1.hours = 0;
	smpte1.minutes = 0;
	smpte1.seconds = 0;
	smpte1.frames = 0;
	smpte1.subframes = 0;
	sample1 = oldsample = 0;
	sample_increment = session->frame_rate() * 60 * 60;

	session->sample_to_smpte( sample1, smpte1, true /* use_offset */, false /* use_subframes */ );
	cout << "Starting at sample: " << sample1 << " -> ";
	cout << "smpte: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << endl;

	for (int i = 0; i < 10; i++) {
		session->smpte_to_sample( smpte1, sample1, true /* use_offset */, false /* use_subframes */ );
		session->sample_to_smpte( sample1, smpte2, true /* use_offset */, false /* use_subframes */ );

//     cout << "smpte: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
//     cout << "sample: " << sample1 << endl;
//     cout << "sample: " << sample1 << " -> ";
//     cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
    
//     if ((i > 0) && ( ((sample1 - oldsample) != sample_increment) && ((sample1 - oldsample) != (sample_increment + 1)) && ((sample1 - oldsample) != (sample_increment - 1))))
//     {
//       cout << "ERROR: sample increment not right: " << (sample1 - oldsample) << " != " << sample_increment << endl;
//       break;
//     }
    
		if (smpte2.hours != smpte1.hours || smpte2.minutes != smpte1.minutes || smpte2.seconds != smpte2.seconds || smpte2.frames != smpte1.frames) {
			cout << "ERROR: smpte2 not equal smpte1" << endl;
			cout << "smpte: " << (smpte1.negative ? "-" : "") << smpte1.hours << ":" << smpte1.minutes << ":" << smpte1.seconds << ":" << smpte1.frames << "::" << smpte1.subframes << " -> ";
			cout << "sample: " << sample1 << endl;
			cout << "sample: " << sample1 << " -> ";
			cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
			break;
		}
		oldsample = sample1;
		session->smpte_increment_hours( smpte1 );
	}

	cout << "sample_increment: " << sample_increment << endl;
	cout << "sample: " << sample1 << " -> ";
	cout << "smpte: " << (smpte2.negative ? "-" : "") << smpte2.hours << ":" << smpte2.minutes << ":" << smpte2.seconds << ":" << smpte2.frames << "::" << smpte2.subframes << endl;
#endif

#endif  

	return sample;
}

nframes_t
AudioClock::minsec_frame_from_display () const
{
	if (session == 0) {
		return 0;
	}

	int hrs = atoi (ms_hours_label.get_text());
	int mins = atoi (ms_minutes_label.get_text());
	float secs = atof (ms_seconds_label.get_text());

	nframes_t sr = session->frame_rate();

	return (nframes_t) floor ((hrs * 60.0f * 60.0f * sr) + (mins * 60.0f * sr) + (secs * sr));
}

nframes_t
AudioClock::bbt_frame_from_display (nframes_t pos) const
{
	if (session == 0) {
		error << "AudioClock::current_time() called with BBT mode but without session!" << endmsg;
		return 0;
	}

	AnyTime any;
	any.type = AnyTime::BBT;

	any.bbt.bars = atoi (bars_label.get_text());
	any.bbt.beats = atoi (beats_label.get_text());
	any.bbt.ticks = atoi (ticks_label.get_text());

       if (is_duration) {
               any.bbt.bars++;
               any.bbt.beats++;
       }

	nframes_t ret = session->convert_to_frames_at (pos, any);

	return ret;
}


nframes_t
AudioClock::bbt_frame_duration_from_display (nframes_t pos) const
{
	if (session == 0) {
		error << "AudioClock::current_time() called with BBT mode but without session!" << endmsg;
		return 0;
	}

	BBT_Time bbt;


	bbt.bars = atoi (bars_label.get_text());
	bbt.beats = atoi (beats_label.get_text());
	bbt.ticks = atoi (ticks_label.get_text());
	
	return session->tempo_map().bbt_duration_at(pos,bbt,1);
}

nframes_t
AudioClock::audio_frame_from_display () const
{
	return (nframes_t) atoi (audio_frames_label.get_text());
}

void
AudioClock::build_ops_menu ()
{
	using namespace Menu_Helpers;
	ops_menu = new Menu;
	MenuList& ops_items = ops_menu->items();
	ops_menu->set_name ("ArdourContextMenu");
	
	if (!Profile->get_sae()) {
		ops_items.push_back (MenuElem (_("Timecode"), bind (mem_fun(*this, &AudioClock::set_mode), SMPTE)));
	}
	ops_items.push_back (MenuElem (_("Bars:Beats"), bind (mem_fun(*this, &AudioClock::set_mode), BBT)));
	ops_items.push_back (MenuElem (_("Minutes:Seconds"), bind (mem_fun(*this, &AudioClock::set_mode), MinSec)));
	ops_items.push_back (MenuElem (_("Samples"), bind (mem_fun(*this, &AudioClock::set_mode), Frames)));
	ops_items.push_back (MenuElem (_("Off"), bind (mem_fun(*this, &AudioClock::set_mode), Off)));
}

void
AudioClock::set_mode (Mode m)
{
	/* slightly tricky: this is called from within the ARDOUR_UI
	   constructor by some of its clock members. at that time
	   the instance pointer is unset, so we have to be careful.
	   the main idea is to drop keyboard focus in case we had
	   started editing the clock and then we switch clock mode.
	*/

	clock_base.grab_focus ();
		
	if (_mode == m) {
		return;
	}
	
	clock_base.remove ();
	
	_mode = m;

	switch (_mode) {
	case SMPTE:
		clock_base.add (smpte_packer_hbox);
		break;

	case BBT:
		clock_base.add (bbt_packer_hbox);
		break;

	case MinSec:
		clock_base.add (minsec_packer_hbox);
		break;

	case Frames:
		clock_base.add (frames_packer_hbox);
		break;

	case Off:
		clock_base.add (off_hbox);
		break;
	}

	set_size_requests ();
	
	set (last_when, true);
	clock_base.show_all ();
	key_entry_state = 0;

	if (!is_transient) {
		ModeChanged (); /* EMIT SIGNAL */
	}
}

void
AudioClock::set_size_requests ()
{
	/* note that in some fonts, "88" is narrower than "00", hence the 2 pixel padding */

	switch (_mode) {
	case SMPTE:
		Gtkmm2ext::set_size_request_to_display_given_text (hours_label, "-00", 5, 5);
		Gtkmm2ext::set_size_request_to_display_given_text (minutes_label, "00", 5, 5);
		Gtkmm2ext::set_size_request_to_display_given_text (seconds_label, "00", 5, 5);
		Gtkmm2ext::set_size_request_to_display_given_text (frames_label, "00", 5, 5);
		break;

	case BBT:
		Gtkmm2ext::set_size_request_to_display_given_text (bars_label, "-000", 5, 5);
		Gtkmm2ext::set_size_request_to_display_given_text (beats_label, "00", 5, 5);
		Gtkmm2ext::set_size_request_to_display_given_text (ticks_label, "0000", 5, 5);
		break;

	case MinSec:
		Gtkmm2ext::set_size_request_to_display_given_text (ms_hours_label, "00", 5, 5);
		Gtkmm2ext::set_size_request_to_display_given_text (ms_minutes_label, "00", 5, 5);
		Gtkmm2ext::set_size_request_to_display_given_text (ms_seconds_label, "00.000", 5, 5);
		break;

	case Frames:
		Gtkmm2ext::set_size_request_to_display_given_text (audio_frames_label, "0000000000", 5, 5);
		break;

	case Off:
		Gtkmm2ext::set_size_request_to_display_given_text (off_hbox, "00000", 5, 5);
		break;
		
	}
}

void
AudioClock::set_bbt_reference (nframes64_t pos)
{
	bbt_reference_time = pos;
}
