/*
    Copyright (C) 2005 Paul Davis 

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

#include <pbd/convert.h>
#include <pbd/stacktrace.h>

#include <gtkmm2ext/utils.h>

#include <ardour/configuration.h>
#include <ardour/session.h>
#include <ardour/osc.h>
#include <ardour/audioengine.h>

#include "ardour_ui.h"
#include "actions.h"
#include "gui_thread.h"
#include "public_editor.h"

#include "i18n.h"

using namespace Gtk;
using namespace Gtkmm2ext;
using namespace ARDOUR;
using namespace PBD;
using namespace sigc;

void
ARDOUR_UI::toggle_time_master ()
{
	ActionManager::toggle_config_state ("Transport", "ToggleTimeMaster", &Configuration::set_jack_time_master, &Configuration::get_jack_time_master);
}

void
ARDOUR_UI::toggle_send_mtc ()
{
	ActionManager::toggle_config_state ("options", "SendMTC", &Configuration::set_send_mtc, &Configuration::get_send_mtc);
}

void
ARDOUR_UI::toggle_send_mmc ()
{
	ActionManager::toggle_config_state ("options", "SendMMC", &Configuration::set_send_mmc, &Configuration::get_send_mmc);
}

void
ARDOUR_UI::toggle_use_mmc ()
{
	ActionManager::toggle_config_state ("options", "UseMMC", &Configuration::set_mmc_control, &Configuration::get_mmc_control);
}

void
ARDOUR_UI::toggle_use_osc ()
{
	ActionManager::toggle_config_state ("options", "UseOSC", &Configuration::set_use_osc, &Configuration::get_use_osc);
}

void
ARDOUR_UI::toggle_seamless_loop ()
{
	ActionManager::toggle_config_state ("options", "toggle-seamless-loop", &Configuration::set_seamless_loop, &Configuration::get_seamless_loop);
}

void
ARDOUR_UI::toggle_send_midi_feedback ()
{
	ActionManager::toggle_config_state ("options", "SendMIDIfeedback", &Configuration::set_midi_feedback, &Configuration::get_midi_feedback);
}

void
ARDOUR_UI::toggle_denormal_protection ()
{
	ActionManager::toggle_config_state ("options", "DenormalProtection", &Configuration::set_denormal_protection, &Configuration::get_denormal_protection);
}

void
ARDOUR_UI::toggle_only_copy_imported_files ()
{
	ActionManager::toggle_config_state ("options", "OnlyCopyImportedFiles", &Configuration::set_only_copy_imported_files, &Configuration::get_only_copy_imported_files);
}

void
ARDOUR_UI::set_native_file_header_format (HeaderFormat hf)
{
	const char *action = 0;

	switch (hf) {
	case BWF:
		action = X_("FileHeaderFormatBWF");
		break;
	case WAVE:
		action = X_("FileHeaderFormatWAVE");
		break;
	case WAVE64:
		action = X_("FileHeaderFormatWAVE64");
		break;
	case iXML:
		action = X_("FileHeaderFormatiXML");
		break;
	case RF64:
		action = X_("FileHeaderFormatRF64");
		break;
	case CAF:
		action = X_("FileHeaderFormatCAF");
		break;
	case AIFF:
		action = X_("FileHeaderFormatAIFF");
		break;
	default:
		fatal << string_compose (_("programming error: %1"), "illegal file header format in ::set_native_file_header_format") << endmsg;
		/*NOTREACHED*/	
	}

	Glib::RefPtr<Action> act = ActionManager::get_action ("options", action);

	if (act) {
		Glib::RefPtr<RadioAction> ract = Glib::RefPtr<RadioAction>::cast_dynamic(act);
		if (ract && ract->get_active() && Config->get_native_file_header_format() != hf) {
			Config->set_native_file_header_format (hf);
		}
	}
}

void
ARDOUR_UI::set_native_file_data_format (SampleFormat sf)
{
	const char* action = 0;

	switch (sf) {
	case FormatFloat:
		action = X_("FileDataFormatFloat");
		break;
	case FormatInt24:
		action = X_("FileDataFormat24bit");
		break;
	case FormatInt16:
		action = X_("FileDataFormat16bit");
		break;
	default:
		fatal << string_compose (_("programming error: %1"), "illegal file data format in ::set_native_file_data_format") << endmsg;
		/*NOTREACHED*/
	}

	Glib::RefPtr<Action> act = ActionManager::get_action ("options", action);

	if (act) {
		Glib::RefPtr<RadioAction> ract = Glib::RefPtr<RadioAction>::cast_dynamic(act);
		if (ract && ract->get_active() && Config->get_native_file_data_format() != sf) {
			Config->set_native_file_data_format (sf);
		}
	}
}

void
ARDOUR_UI::set_input_auto_connect (AutoConnectOption option)
{
	const char* action;
	
	switch (option) {
	case AutoConnectPhysical:
		action = X_("InputAutoConnectPhysical");
		break;
	default:
		action = X_("InputAutoConnectManual");
	}

	Glib::RefPtr<Action> act = ActionManager::get_action ("options", action);

	if (act) {
		Glib::RefPtr<RadioAction> ract = Glib::RefPtr<RadioAction>::cast_dynamic(act);

		if (ract && ract->get_active() && Config->get_input_auto_connect() != option) {
			Config->set_input_auto_connect (option);
		}
	}
}

void
ARDOUR_UI::set_output_auto_connect (AutoConnectOption option)
{
	const char* action;
	
	switch (option) {
	case AutoConnectPhysical:
		action = X_("OutputAutoConnectPhysical");
		break;
	case AutoConnectMaster:
		action = X_("OutputAutoConnectMaster");
		break;
	default:
		action = X_("OutputAutoConnectManual");
	}

	Glib::RefPtr<Action> act = ActionManager::get_action ("options", action);

	if (act) {
		Glib::RefPtr<RadioAction> ract = Glib::RefPtr<RadioAction>::cast_dynamic(act);

		if (ract && ract->get_active() && Config->get_output_auto_connect() != option) {
			Config->set_output_auto_connect (option);
		}
	}
}

void
ARDOUR_UI::set_solo_model (SoloModel model)
{
	const char* action = 0;

	switch (model) {
	case SoloBus:
		action = X_("SoloViaBus");
		break;
		
	case InverseMute:
		action = X_("SoloInPlace");
		break;
	default:
		fatal << string_compose (_("programming error: unknown solo model in ARDOUR_UI::set_solo_model: %1"), model) << endmsg;
		/*NOTREACHED*/
	}

	Glib::RefPtr<Action> act = ActionManager::get_action ("options", action);

	if (act) {
		Glib::RefPtr<RadioAction> ract = Glib::RefPtr<RadioAction>::cast_dynamic(act);

		if (ract && ract->get_active() && Config->get_solo_model() != model) {
			Config->set_solo_model (model);
		}
	}

}

void
ARDOUR_UI::set_remote_model (RemoteModel model)
{
	const char* action = 0;

	switch (model) {
	case UserOrdered:
		action = X_("RemoteUserDefined");
		break;
	case MixerOrdered:
		action = X_("RemoteMixerDefined");
		break;
	case EditorOrdered:
		action = X_("RemoteEditorDefined");
		break;

	default:
		fatal << string_compose (_("programming error: unknown remote model in ARDOUR_UI::set_remote_model: %1"), model) << endmsg;
		/*NOTREACHED*/
	}

	Glib::RefPtr<Action> act = ActionManager::get_action ("options", action);

	if (act) {
		Glib::RefPtr<RadioAction> ract = Glib::RefPtr<RadioAction>::cast_dynamic(act);

		if (ract && ract->get_active() && Config->get_remote_model() != model) {
			Config->set_remote_model (model);
		}
	}

}

void
ARDOUR_UI::set_monitor_model (MonitorModel model)
{
	const char* action = 0;

	switch (model) {
	case HardwareMonitoring:
		action = X_("UseHardwareMonitoring");
		break;
		
	case SoftwareMonitoring:
		action = X_("UseSoftwareMonitoring");
		break;
	case ExternalMonitoring:
		action = X_("UseExternalMonitoring");
		break;

	default:
		fatal << string_compose (_("programming error: unknown monitor model in ARDOUR_UI::set_monitor_model: %1"), model) << endmsg;
		/*NOTREACHED*/
	}

	Glib::RefPtr<Action> act = ActionManager::get_action ("options", action);

	if (act) {
		Glib::RefPtr<RadioAction> ract = Glib::RefPtr<RadioAction>::cast_dynamic(act);

		if (ract && ract->get_active() && Config->get_monitoring_model() != model) {
			Config->set_monitoring_model (model);
		}
	}

}

void
ARDOUR_UI::set_denormal_model (DenormalModel model)
{
	const char* action = 0;

	switch (model) {
	case DenormalNone:
		action = X_("DenormalNone");
		break;

	case DenormalFTZ:
		action = X_("DenormalFTZ");
		break;
		
	case DenormalDAZ:
		action = X_("DenormalDAZ");
		break;

	case DenormalFTZDAZ:
		action = X_("DenormalFTZDAZ");
		break;

	default:
		fatal << string_compose (_("programming error: unknown denormal model in ARDOUR_UI::set_denormal_model: %1"), model) << endmsg;
		/*NOTREACHED*/
	}

	Glib::RefPtr<Action> act = ActionManager::get_action ("options", action);

	if (act) {
		Glib::RefPtr<RadioAction> ract = Glib::RefPtr<RadioAction>::cast_dynamic(act);

		if (ract && ract->get_active() && Config->get_denormal_model() != model) {
			Config->set_denormal_model (model);
		}
	}

}

void
ARDOUR_UI::toggle_auto_input ()
{
	ActionManager::toggle_config_state ("Transport", "ToggleAutoInput", &Configuration::set_auto_input, &Configuration::get_auto_input);
}

void
ARDOUR_UI::toggle_auto_play ()
{
	ActionManager::toggle_config_state ("Transport", "ToggleAutoPlay", &Configuration::set_auto_play, &Configuration::get_auto_play);
}

void
ARDOUR_UI::toggle_auto_return ()
{
	ActionManager::toggle_config_state ("Transport", "ToggleAutoReturn", &Configuration::set_auto_return, &Configuration::get_auto_return);
}

void
ARDOUR_UI::toggle_click ()
{
	ActionManager::toggle_config_state ("Transport", "ToggleClick", &Configuration::set_clicking, &Configuration::get_clicking);
}

void
ARDOUR_UI::unset_dual_punch ()
{
	Glib::RefPtr<Action> action = ActionManager::get_action ("Transport", "TogglePunch");
	
	if (action) {
		Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic(action);
		if (tact) {
			ignore_dual_punch = true;
			tact->set_active (false);
			ignore_dual_punch = false;
		}
	}
}

void
ARDOUR_UI::toggle_punch ()
{
	if (ignore_dual_punch) {
		return;
	}

	Glib::RefPtr<Action> action = ActionManager::get_action ("Transport", "TogglePunch");

	if (action) {

		Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic(action);

		if (!tact) {
			return;
		}

		/* drive the other two actions from this one */

		Glib::RefPtr<Action> in_action = ActionManager::get_action ("Transport", "TogglePunchIn");
		Glib::RefPtr<Action> out_action = ActionManager::get_action ("Transport", "TogglePunchOut");

		if (in_action && out_action) {
			Glib::RefPtr<ToggleAction> tiact = Glib::RefPtr<ToggleAction>::cast_dynamic(in_action);
			Glib::RefPtr<ToggleAction> toact = Glib::RefPtr<ToggleAction>::cast_dynamic(out_action);
			tiact->set_active (tact->get_active());
			toact->set_active (tact->get_active());
		}
	}
}

void
ARDOUR_UI::toggle_punch_in ()
{
	Glib::RefPtr<Action> act = ActionManager::get_action (X_("Transport"), X_("TogglePunchIn"));
	if (!act) {
		return;
	}

	Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic (act);
	if (!tact) {
		return;
	}

	if (tact->get_active() != Config->get_punch_in()) {
		Config->set_punch_in (tact->get_active ());
	}

	if (tact->get_active()) {
		/* if punch-in is turned on, make sure the loop/punch ruler is visible, and stop it being hidden,
		   to avoid confusing the user */
		show_loop_punch_ruler_and_disallow_hide ();
	}

	reenable_hide_loop_punch_ruler_if_appropriate ();
}

void
ARDOUR_UI::toggle_punch_out ()
{
	Glib::RefPtr<Action> act = ActionManager::get_action (X_("Transport"), X_("TogglePunchOut"));
	if (!act) {
		return;
	}

	Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic (act);
	if (!tact) {
		return;
	}

	if (tact->get_active() != Config->get_punch_out()) {
		Config->set_punch_out (tact->get_active ());
	}

	if (tact->get_active()) {
		/* if punch-out is turned on, make sure the loop/punch ruler is visible, and stop it being hidden,
		   to avoid confusing the user */
		show_loop_punch_ruler_and_disallow_hide ();
	}
	reenable_hide_loop_punch_ruler_if_appropriate ();
}

void
ARDOUR_UI::show_loop_punch_ruler_and_disallow_hide ()
{
	Glib::RefPtr<Action> act = ActionManager::get_action (X_("Rulers"), "toggle-loop-punch-ruler");
	if (!act) {
		return;
	}

	act->set_sensitive (false);
	
	Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic (act);
	if (!tact) {
		return;
	}
	
	if (!tact->get_active()) {
		tact->set_active ();
	}
}

/* This is a bit of a silly name for a method */
void
ARDOUR_UI::reenable_hide_loop_punch_ruler_if_appropriate ()
{
	if (!Config->get_punch_in() && !Config->get_punch_out()) {
		/* if punch in/out are now both off, reallow hiding of the loop/punch ruler */
		Glib::RefPtr<Action> act = ActionManager::get_action (X_("Rulers"), "toggle-loop-punch-ruler");
		if (act) {
			act->set_sensitive (true);
		}
	}
}

void
ARDOUR_UI::toggle_video_sync()
{
	Glib::RefPtr<Action> act = ActionManager::get_action ("Transport", "ToggleVideoSync");
	if (act) {
		Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic(act);
		Config->set_use_video_sync (tact->get_active());
	}
}

void
ARDOUR_UI::toggle_editing_space()
{
	Glib::RefPtr<Action> act = ActionManager::get_action ("Common", "ToggleMaximalEditor");
	if (act) {
		Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic(act);
		if (tact->get_active()) {
			maximise_editing_space ();
		} else {
			restore_editing_space ();
		}
	}
}

void
ARDOUR_UI::toggle_new_plugins_active ()
{
	ActionManager::toggle_config_state ("options", "NewPluginsActive", &Configuration::set_new_plugins_active, &Configuration::get_new_plugins_active);
}

void
ARDOUR_UI::toggle_StopPluginsWithTransport()
{
	ActionManager::toggle_config_state ("options", "StopPluginsWithTransport", &Configuration::set_plugins_stop_with_transport, &Configuration::get_plugins_stop_with_transport);
}

void
ARDOUR_UI::toggle_LatchedRecordEnable()
{
	ActionManager::toggle_config_state ("options", "LatchedRecordEnable", &Configuration::set_latched_record_enable, &Configuration::get_latched_record_enable);
}

void
ARDOUR_UI::toggle_RegionEquivalentsOverlap()
{
	ActionManager::toggle_config_state ("options", "RegionEquivalentsOverlap", &Configuration::set_use_overlap_equivalency, &Configuration::get_use_overlap_equivalency);
}

void
ARDOUR_UI::toggle_DoNotRunPluginsWhileRecording()
{
	ActionManager::toggle_config_state ("options", "DoNotRunPluginsWhileRecording", &Configuration::set_do_not_record_plugins, &Configuration::get_do_not_record_plugins);
}

void
ARDOUR_UI::toggle_VerifyRemoveLastCapture()
{
	ActionManager::toggle_config_state ("options", "VerifyRemoveLastCapture", &Configuration::set_verify_remove_last_capture, &Configuration::get_verify_remove_last_capture);
}

void
ARDOUR_UI::toggle_PeriodicSafetyBackups()
{
	ActionManager::toggle_config_state ("options", "PeriodicSafetyBackups", &Configuration::set_periodic_safety_backups, &Configuration::get_periodic_safety_backups);
}

void
ARDOUR_UI::toggle_StopRecordingOnXrun()
{
	ActionManager::toggle_config_state ("options", "StopRecordingOnXrun", &Configuration::set_stop_recording_on_xrun, &Configuration::get_stop_recording_on_xrun);
}

void
ARDOUR_UI::toggle_CreateXrunMarker()
{
	ActionManager::toggle_config_state ("options", "CreateXrunMarker", &Configuration::set_create_xrun_marker, &Configuration::get_create_xrun_marker);
}

void
ARDOUR_UI::toggle_sync_order_keys ()
{
	ActionManager::toggle_config_state ("options", "SyncEditorAndMixerTrackOrder", &Configuration::set_sync_all_route_ordering, &Configuration::get_sync_all_route_ordering);
}

void
ARDOUR_UI::toggle_StopTransportAtEndOfSession()
{
	ActionManager::toggle_config_state ("options", "StopTransportAtEndOfSession", &Configuration::set_stop_at_session_end, &Configuration::get_stop_at_session_end);
}

void
ARDOUR_UI::toggle_GainReduceFastTransport()
{
	ActionManager::toggle_config_state ("options", "GainReduceFastTransport", &Configuration::set_quieten_at_speed, &Configuration::get_quieten_at_speed);
}

void
ARDOUR_UI::toggle_LatchedSolo()
{
	ActionManager::toggle_config_state ("options", "LatchedSolo", &Configuration::set_solo_latched, &Configuration::get_solo_latched);
}

void
ARDOUR_UI::toggle_ShowSoloMutes()
{
	ActionManager::toggle_config_state ("options", "ShowSoloMutes", &Configuration::set_show_solo_mutes, &Configuration::get_show_solo_mutes);
}

void
ARDOUR_UI::toggle_SoloMuteOverride()
{
	ActionManager::toggle_config_state ("options", "SoloMuteOverride", &Configuration::set_solo_mute_override, &Configuration::get_solo_mute_override);
}

void
ARDOUR_UI::toggle_PrimaryClockDeltaEditCursor()
{
	ActionManager::toggle_config_state ("options", "PrimaryClockDeltaEditCursor", &Configuration::set_primary_clock_delta_edit_cursor, &Configuration::get_primary_clock_delta_edit_cursor);
}

void
ARDOUR_UI::toggle_SecondaryClockDeltaEditCursor()
{
	ActionManager::toggle_config_state ("options", "SecondaryClockDeltaEditCursor", &Configuration::set_secondary_clock_delta_edit_cursor, &Configuration::get_secondary_clock_delta_edit_cursor);
}

void
ARDOUR_UI::toggle_ShowTrackMeters()
{
	ActionManager::toggle_config_state ("options", "ShowTrackMeters", &Configuration::set_show_track_meters, &Configuration::get_show_track_meters);
}

void
ARDOUR_UI::toggle_TapeMachineMode ()
{
	ActionManager::toggle_config_state ("options", "ToggleTapeMachineMode", &Configuration::set_tape_machine_mode, &Configuration::get_tape_machine_mode);
}

void
ARDOUR_UI::toggle_use_narrow_ms()
{
	ActionManager::toggle_config_state ("options", "DefaultNarrowMS", &Configuration::set_default_narrow_ms, &Configuration::get_default_narrow_ms);
}

void
ARDOUR_UI::toggle_NameNewMarkers()
{
	ActionManager::toggle_config_state ("options", "NameNewMarkers", &Configuration::set_name_new_markers, &Configuration::get_name_new_markers);
}

void
ARDOUR_UI::toggle_rubberbanding_snaps_to_grid ()
{
	ActionManager::toggle_config_state ("options", "RubberbandingSnapsToGrid", &Configuration::set_rubberbanding_snaps_to_grid, &Configuration::get_rubberbanding_snaps_to_grid);
}

void
ARDOUR_UI::toggle_auto_analyse_audio ()
{
	ActionManager::toggle_config_state ("options", "AutoAnalyseAudio", &Configuration::set_auto_analyse_audio, &Configuration::get_auto_analyse_audio);
}

void
ARDOUR_UI::mtc_port_changed ()
{
	bool have_mtc;

	if (session) {
		if (session->mtc_port()) {
			have_mtc = true;
		} else {
			have_mtc = false;
		}
	} else {
		have_mtc = false;
	}

	positional_sync_strings.clear ();
	positional_sync_strings.push_back (slave_source_to_string (None));
	if (have_mtc) {
		positional_sync_strings.push_back (slave_source_to_string (MTC));
	}
	positional_sync_strings.push_back (slave_source_to_string (JACK));

	set_popdown_strings (sync_option_combo, positional_sync_strings);
}

void
ARDOUR_UI::setup_session_options ()
{
	mtc_port_changed ();

	Config->ParameterChanged.connect (mem_fun (*this, &ARDOUR_UI::parameter_changed));
}


void
ARDOUR_UI::map_solo_model ()
{
	const char* on;

	if (Config->get_solo_model() == InverseMute) {
		on = X_("SoloInPlace");
	} else {
		on = X_("SoloViaBus");
	}

	Glib::RefPtr<Action> act = ActionManager::get_action ("options", on);
	if (act) {
		Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic(act);

		if (tact && !tact->get_active()) {
			tact->set_active (true);
		}
	}
}

void
ARDOUR_UI::map_monitor_model ()
{
	const char* on = 0;

	switch (Config->get_monitoring_model()) {
	case HardwareMonitoring:
		on = X_("UseHardwareMonitoring");
		break;
	case SoftwareMonitoring:
		on = X_("UseSoftwareMonitoring");
		break;
	case ExternalMonitoring:
		on = X_("UseExternalMonitoring");
		break;
	}

	Glib::RefPtr<Action> act = ActionManager::get_action ("options", on);
	if (act) {
		Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic(act);

		if (tact && !tact->get_active()) {
			tact->set_active (true);
		}
	}
}

void
ARDOUR_UI::map_denormal_protection ()
{
	Glib::RefPtr<Action> act = ActionManager::get_action ("options", X_("DenormalProtection"));
	if (act) {
		Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic(act);

		if (tact && !tact->get_active()) {
			tact->set_active (Config->get_denormal_protection());
		}
	}
}

void
ARDOUR_UI::map_denormal_model ()
{
	const char* on = 0;

	switch (Config->get_denormal_model()) {
	case DenormalNone:
		on = X_("DenormalNone");
		break;
	case DenormalFTZ:
		on = X_("DenormalFTZ");
		break;
	case DenormalDAZ:
		on = X_("DenormalDAZ");
		break;
	case DenormalFTZDAZ:
		on = X_("DenormalFTZDAZ");
		break;
	}

	Glib::RefPtr<Action> act = ActionManager::get_action ("options", on);
	if (act) {
		Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic(act);

		if (tact && !tact->get_active()) {
			tact->set_active (true);
		}
	}
}

void
ARDOUR_UI::map_remote_model ()
{
	const char* on = 0;

	switch (Config->get_remote_model()) {
	case UserOrdered:
		on = X_("RemoteUserDefined");
		break;
	case MixerOrdered:
		on = X_("RemoteMixerDefined");
		break;
	case EditorOrdered:
		on = X_("RemoteEditorDefined");
		break;
	}

	Glib::RefPtr<Action> act = ActionManager::get_action ("options", on);
	if (act) {
		Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic(act);

		if (tact && !tact->get_active()) {
			tact->set_active (true);
		}
	}
}

void
ARDOUR_UI::map_file_header_format ()
{
	const char* action = 0;

	switch (Config->get_native_file_header_format()) {
	case BWF:
		action = X_("FileHeaderFormatBWF");
		break;

	case WAVE:
		action = X_("FileHeaderFormatWAVE");
		break;

	case WAVE64:
		action = X_("FileHeaderFormatWAVE64");
		break;

	case iXML:
		action = X_("FileHeaderFormatiXML");
		break;

	case RF64:
		action = X_("FileHeaderFormatRF64");
		break;

	case CAF:
		action = X_("FileHeaderFormatCAF");
		break;

	default:
		fatal << string_compose (_("programming error: unknown file header format passed to ARDOUR_UI::map_file_data_format: %1"), 
					 Config->get_native_file_header_format()) << endmsg;
		/*NOTREACHED*/
	}


	Glib::RefPtr<Action> act = ActionManager::get_action ("options", action);

	if (act) {
		Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic(act);

		if (tact && !tact->get_active()) {
			tact->set_active (true);
		}
	}
}

void
ARDOUR_UI::map_file_data_format ()
{
	const char* action = 0;

	switch (Config->get_native_file_data_format()) {
	case FormatFloat:
		action = X_("FileDataFormatFloat");
		break;

	case FormatInt24:
		action = X_("FileDataFormat24bit");
		break;

	case FormatInt16:
		action = X_("FileDataFormat16bit");
		break;

	default:
		fatal << string_compose (_("programming error: unknown file data format passed to ARDOUR_UI::map_file_data_format: %1"), 
					 Config->get_native_file_data_format()) << endmsg;
		/*NOTREACHED*/
	}


	Glib::RefPtr<Action> act = ActionManager::get_action ("options", action);

	if (act) {
		Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic(act);

		if (tact && !tact->get_active()) {
			tact->set_active (true);
		}
	}
}

void
ARDOUR_UI::map_input_auto_connect ()
{
	const char* on;

	if (Config->get_input_auto_connect() == (AutoConnectOption) 0) {
		on = "InputAutoConnectManual";
	} else {
		on = "InputAutoConnectPhysical";
	}

	Glib::RefPtr<Action> act = ActionManager::get_action ("options", on);
	if (act) {
		Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic(act);

		if (tact && !tact->get_active()) {
			tact->set_active (true);
		}
	}
}

void
ARDOUR_UI::map_output_auto_connect ()
{
	const char* on;

	if (Config->get_output_auto_connect() == (AutoConnectOption) 0) {
		on = "OutputAutoConnectManual";
	} else if (Config->get_output_auto_connect() == AutoConnectPhysical) {
		on = "OutputAutoConnectPhysical";
	} else {
		on = "OutputAutoConnectMaster";
	}

	Glib::RefPtr<Action> act = ActionManager::get_action ("options", on);
	if (act) {
		Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic(act);
		
		if (tact && !tact->get_active()) {
			tact->set_active (true);
		}
	}
}

void
ARDOUR_UI::map_only_copy_imported_files ()
{
	Glib::RefPtr<Action> act = ActionManager::get_action ("options", X_("OnlyCopyImportedFiles"));
	if (act) {
		Glib::RefPtr<ToggleAction> tact = Glib::RefPtr<ToggleAction>::cast_dynamic(act);

		if (tact && !tact->get_active()) {
			tact->set_active (Config->get_only_copy_imported_files());
		}
	}

}

void
ARDOUR_UI::map_meter_falloff ()
{
	const char* action = X_("MeterFalloffMedium");

	float val = Config->get_meter_falloff ();
	MeterFalloff code = meter_falloff_from_float(val);

	switch (code) {
	case MeterFalloffOff:
		action = X_("MeterFalloffOff");
		break;
	case MeterFalloffSlowest:
		action = X_("MeterFalloffSlowest");
		break;
	case MeterFalloffSlow:
		action = X_("MeterFalloffSlow");
		break;
	case MeterFalloffMedium:
		action = X_("MeterFalloffMedium");
		break;
	case MeterFalloffFast:
		action = X_("MeterFalloffFast");
		break;
	case MeterFalloffFaster:
		action = X_("MeterFalloffFaster");
		break;
	case MeterFalloffFastest:
		action = X_("MeterFalloffFastest");
		break;
	}

	Glib::RefPtr<Action> act = ActionManager::get_action (X_("options"), action);

	if (act) {
		Glib::RefPtr<RadioAction> ract = Glib::RefPtr<RadioAction>::cast_dynamic(act);
		if (ract && !ract->get_active()) {
			ract->set_active (true);
		}
	}
}

void
ARDOUR_UI::map_meter_hold ()
{
	const char* action = X_("MeterHoldMedium");

	/* XXX hack alert. Fix this. Please */

	float val = Config->get_meter_hold ();
	MeterHold code = (MeterHold) (int) (floor (val));

	switch (code) {
	case MeterHoldOff:
		action = X_("MeterHoldOff");
		break;
	case MeterHoldShort:
		action = X_("MeterHoldShort");
		break;
	case MeterHoldMedium:
		action = X_("MeterHoldMedium");
		break;
	case MeterHoldLong:
		action = X_("MeterHoldLong");
		break;
	}

	Glib::RefPtr<Action> act = ActionManager::get_action (X_("options"), action);

	if (act) {
		Glib::RefPtr<RadioAction> ract = Glib::RefPtr<RadioAction>::cast_dynamic(act);
		if (ract && !ract->get_active()) {
			ract->set_active (true);
		}
	}
}

void 
ARDOUR_UI::set_meter_hold (MeterHold val)
{
	const char* action = 0;
	float fval;

	fval = meter_hold_to_float (val);

	switch (val) {
	case MeterHoldOff:
		action = X_("MeterHoldOff");
		break;
	case MeterHoldShort:
		action = X_("MeterHoldShort");
		break;
	case MeterHoldMedium:
		action = X_("MeterHoldMedium");
		break;
	case MeterHoldLong:
		action = X_("MeterHoldLong");
		break;
	}

	Glib::RefPtr<Action> act = ActionManager::get_action (X_("options"), action);
	
	if (act) {
		Glib::RefPtr<RadioAction> ract = Glib::RefPtr<RadioAction>::cast_dynamic(act);
		if (ract && ract->get_active() && Config->get_meter_hold() != fval) {
			Config->set_meter_hold (fval);
		}
	}
}

void
ARDOUR_UI::set_meter_falloff (MeterFalloff val)
{
	const char* action = 0;
	float fval;

	fval = meter_falloff_to_float (val);

	switch (val) {
	case MeterFalloffOff:
		action = X_("MeterFalloffOff");
		break;
	case MeterFalloffSlowest:
		action = X_("MeterFalloffSlowest");
		break;
	case MeterFalloffSlow:
		action = X_("MeterFalloffSlow");
		break;
	case MeterFalloffMedium:
		action = X_("MeterFalloffMedium");
		break;
	case MeterFalloffFast:
		action = X_("MeterFalloffFast");
		break;
	case MeterFalloffFaster:
		action = X_("MeterFalloffFaster");
		break;
	case MeterFalloffFastest:
		action = X_("MeterFalloffFastest");
		break;
	}

	Glib::RefPtr<Action> act = ActionManager::get_action (X_("options"), action);

	if (act) {
		Glib::RefPtr<RadioAction> ract = Glib::RefPtr<RadioAction>::cast_dynamic(act);
		if (ract && ract->get_active() && Config->get_meter_falloff () != fval) {
			Config->set_meter_falloff (fval);
		}
	}
}

void
ARDOUR_UI::parameter_changed (const char* parameter_name)
{
	ENSURE_GUI_THREAD (bind (mem_fun (*this, &ARDOUR_UI::parameter_changed), parameter_name));

#define PARAM_IS(x) (!strcmp (parameter_name, (x)))
	
	if (PARAM_IS ("slave-source")) {

		sync_option_combo.set_active_text (slave_source_to_string (Config->get_slave_source()));
		
		switch (Config->get_slave_source()) {
		case None:
			ActionManager::get_action ("Transport", "ToggleAutoPlay")->set_sensitive (true);
			ActionManager::get_action ("Transport", "ToggleAutoReturn")->set_sensitive (true);
			break;

		default:
			/* XXX need to make auto-play is off as well as insensitive */
			ActionManager::get_action ("Transport", "ToggleAutoPlay")->set_sensitive (false);
			ActionManager::get_action ("Transport", "ToggleAutoReturn")->set_sensitive (false);
			break;
		}

	} else if (PARAM_IS ("send-mtc")) {

		ActionManager::map_some_state ("options", "SendMTC", &Configuration::get_send_mtc);

	} else if (PARAM_IS ("send-mmc")) {


		ActionManager::map_some_state ("options", "SendMMC", &Configuration::get_send_mmc);

	} else if (PARAM_IS ("use-osc")) {

#ifdef HAVE_LIBLO
		if (Config->get_use_osc()) {
			osc->start ();
		} else {
			osc->stop ();
		}
#endif

		ActionManager::map_some_state ("options", "UseOSC", &Configuration::get_use_osc);
		
	} else if (PARAM_IS ("seamless-loop")) {
		ActionManager::map_some_state ("options", "toggle-seamless-loop", &Configuration::get_seamless_loop);

	} else if (PARAM_IS ("mmc-control")) {
		ActionManager::map_some_state ("options", "UseMMC", &Configuration::get_mmc_control);

	} else if (PARAM_IS ("midi-feedback")) {
		ActionManager::map_some_state ("options", "SendMIDIfeedback", &Configuration::get_midi_feedback);
	} else if (PARAM_IS ("do-not-record-plugins")) {
		ActionManager::map_some_state ("options", "DoNotRunPluginsWhileRecording", &Configuration::get_do_not_record_plugins);
	} else if (PARAM_IS ("latched-record-enable")) {
		ActionManager::map_some_state ("options", "LatchedRecordEnable", &Configuration::get_latched_record_enable);
	} else if (PARAM_IS ("solo-latched")) {
		ActionManager::map_some_state ("options", "LatchedSolo", &Configuration::get_solo_latched);
	} else if (PARAM_IS ("show-solo-mutes")) {
		ActionManager::map_some_state ("options", "ShowSoloMutes", &Configuration::get_show_solo_mutes);
	} else if (PARAM_IS ("solo-mute-override")) {
		ActionManager::map_some_state ("options", "SoloMuteOverride", &Configuration::get_solo_mute_override);
	} else if (PARAM_IS ("solo-model")) {
		map_solo_model ();
	} else if (PARAM_IS ("auto-play")) {
		ActionManager::map_some_state ("Transport", "ToggleAutoPlay", &Configuration::get_auto_play);
	} else if (PARAM_IS ("auto-return")) {
		ActionManager::map_some_state ("Transport", "ToggleAutoReturn", &Configuration::get_auto_return);
	} else if (PARAM_IS ("auto-input")) {
		ActionManager::map_some_state ("Transport", "ToggleAutoInput", &Configuration::get_auto_input);
	} else if (PARAM_IS ("tape-machine-mode")) {
		ActionManager::map_some_state ("options", "ToggleTapeMachineMode", &Configuration::get_tape_machine_mode);
	} else if (PARAM_IS ("punch-out")) {
		ActionManager::map_some_state ("Transport", "TogglePunchOut", &Configuration::get_punch_out);
		if (!Config->get_punch_out()) {
			unset_dual_punch ();
		}
	} else if (PARAM_IS ("punch-in")) {
		ActionManager::map_some_state ("Transport", "TogglePunchIn", &Configuration::get_punch_in);
		if (!Config->get_punch_in()) {
			unset_dual_punch ();
		}
	} else if (PARAM_IS ("clicking")) {
		ActionManager::map_some_state ("Transport", "ToggleClick", &Configuration::get_clicking);
	} else if (PARAM_IS ("jack-time-master")) {
		ActionManager::map_some_state ("Transport",  "ToggleTimeMaster", &Configuration::get_jack_time_master);
	} else if (PARAM_IS ("plugins-stop-with-transport")) {
		ActionManager::map_some_state ("options",  "StopPluginsWithTransport", &Configuration::get_plugins_stop_with_transport);
	} else if (PARAM_IS ("new-plugins-active")) {
		ActionManager::map_some_state ("options",  "NewPluginsActive", &Configuration::get_new_plugins_active);
	} else if (PARAM_IS ("latched-record-enable")) {
		ActionManager::map_some_state ("options", "LatchedRecordEnable", &Configuration::get_latched_record_enable);
	} else if (PARAM_IS ("verify-remove-last-capture")) {
		ActionManager::map_some_state ("options",  "VerifyRemoveLastCapture", &Configuration::get_verify_remove_last_capture);
	} else if (PARAM_IS ("periodic-safety-backups")) {
		ActionManager::map_some_state ("options",  "PeriodicSafetyBackups", &Configuration::get_periodic_safety_backups);
	} else if (PARAM_IS ("stop-recording-on-xrun")) {
		ActionManager::map_some_state ("options",  "StopRecordingOnXrun", &Configuration::get_stop_recording_on_xrun);
	} else if (PARAM_IS ("create-xrun-marker")) {
		ActionManager::map_some_state ("options",  "CreateXrunMarker", &Configuration::get_create_xrun_marker);
	} else if (PARAM_IS ("sync-all-route-ordering")) {
		ActionManager::map_some_state ("options",  "SyncEditorAndMixerTrackOrder", &Configuration::get_sync_all_route_ordering);
	} else if (PARAM_IS ("stop-at-session-end")) {
		ActionManager::map_some_state ("options",  "StopTransportAtEndOfSession", &Configuration::get_stop_at_session_end);
	} else if (PARAM_IS ("monitoring-model")) {
		map_monitor_model ();
	} else if (PARAM_IS ("denormal-model")) {
		map_denormal_model ();
	} else if (PARAM_IS ("denormal-protection")) {
		map_denormal_protection ();
	} else if (PARAM_IS ("remote-model")) {
		map_remote_model ();
	} else if (PARAM_IS ("use-video-sync")) {
		ActionManager::map_some_state ("Transport",  "ToggleVideoSync", &Configuration::get_use_video_sync);
	} else if (PARAM_IS ("quieten-at-speed")) {
		ActionManager::map_some_state ("options",  "GainReduceFastTransport", &Configuration::get_quieten_at_speed);
	} else if (PARAM_IS ("shuttle-behaviour")) {

		switch (Config->get_shuttle_behaviour ()) {
		case Sprung:
			shuttle_style_button.set_active_text (_("sprung"));
			shuttle_fract = 0.0;
			shuttle_box.queue_draw ();
			if (session) {
				if (session->transport_rolling()) {
					shuttle_fract = SHUTTLE_FRACT_SPEED1;
					session->request_transport_speed (1.0);
				}
			}
			break;
		case Wheel:
			shuttle_style_button.set_active_text (_("wheel"));
			break;
		}

	} else if (PARAM_IS ("shuttle-units")) {
		
		switch (Config->get_shuttle_units()) {
		case Percentage:
			shuttle_units_button.set_label("% ");
			break;
		case Semitones:
			shuttle_units_button.set_label(_("ST"));
			break;
		}
	} else if (PARAM_IS ("input-auto-connect")) {
		map_input_auto_connect ();
	} else if (PARAM_IS ("output-auto-connect")) {
		map_output_auto_connect ();
	} else if (PARAM_IS ("native-file-header-format")) {
		map_file_header_format ();
	} else if (PARAM_IS ("native-file-data-format")) {
		map_file_data_format ();
	} else if (PARAM_IS ("meter-hold")) {
		map_meter_hold ();
	} else if (PARAM_IS ("meter-falloff")) {
		map_meter_falloff ();
	} else if (PARAM_IS ("video-pullup") || PARAM_IS ("smpte-format")) {
		if (session) {
			primary_clock.set (session->audible_frame(), true);
			secondary_clock.set (session->audible_frame(), true);
		} else {
			primary_clock.set (0, true);
			secondary_clock.set (0, true);
		}
	} else if (PARAM_IS ("use-overlap-equivalency")) {
		ActionManager::map_some_state ("options", "RegionEquivalentsOverlap", &Configuration::get_use_overlap_equivalency);
	} else if (PARAM_IS ("primary-clock-delta-edit-cursor")) {
		ActionManager::map_some_state ("options",  "PrimaryClockDeltaEditCursor", &Configuration::get_primary_clock_delta_edit_cursor);
	} else if (PARAM_IS ("secondary-clock-delta-edit-cursor")) {
		ActionManager::map_some_state ("options",  "SecondaryClockDeltaEditCursor", &Configuration::get_secondary_clock_delta_edit_cursor);
	} else if (PARAM_IS ("only-copy-imported-files")) {
		map_only_copy_imported_files ();
	} else if (PARAM_IS ("show-track-meters")) {
		ActionManager::map_some_state ("options",  "ShowTrackMeters", &Configuration::get_show_track_meters);
		editor->toggle_meter_updating();
	} else if (PARAM_IS ("default-narrow_ms")) {
		ActionManager::map_some_state ("options",  "DefaultNarrowMS", &Configuration::get_default_narrow_ms);
	} else if (PARAM_IS ("name-new-markers")) {
		ActionManager::map_some_state ("options",  "NameNewMarkers", &Configuration::get_name_new_markers);	
	} else if (PARAM_IS ("rubberbanding-snaps-to-grid")) {
		ActionManager::map_some_state ("options", "RubberbandingSnapsToGrid", &Configuration::get_rubberbanding_snaps_to_grid);
	}

			   

#undef PARAM_IS
}
