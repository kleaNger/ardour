/*
    Copyright (C) 2006 Paul Davis 

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

#ifndef __ardour_track_h__
#define __ardour_track_h__

#include <boost/shared_ptr.hpp>

#include <ardour/route.h>

namespace ARDOUR {

class Session;
class Diskstream;
class Playlist;
class RouteGroup;

class Track : public Route
{
  public:
	Track (Session&, string name, Route::Flag f = Route::Flag (0), TrackMode m = Normal, DataType default_type = DataType::AUDIO);

	virtual ~Track ();
	
	int set_name (string str, void *src);

	TrackMode mode () const { return _mode; }
	virtual int set_mode (TrackMode m) { return false; }
	virtual bool can_use_mode (TrackMode m, bool& bounce_required) { return false; }
	sigc::signal<void> TrackModeChanged;

	virtual int roll (nframes_t nframes, nframes_t start_frame, nframes_t end_frame, 
			  int declick, bool can_record, bool rec_monitors_input) = 0;
	
	virtual int no_roll (nframes_t nframes, nframes_t start_frame, nframes_t end_frame, 
			     bool state_changing, bool can_record, bool rec_monitors_input) = 0;
	
	virtual int silent_roll (nframes_t nframes, nframes_t start_frame, nframes_t end_frame, 
				 bool can_record, bool rec_monitors_input) = 0;

	void toggle_monitor_input ();

	virtual bool can_record();

	boost::shared_ptr<Diskstream> diskstream() const { return _diskstream; }

	virtual int use_diskstream (string name) = 0;
	virtual int use_diskstream (const PBD::ID& id) = 0;

	void      set_latency_compensation (nframes_t);

	enum FreezeState {
		NoFreeze,
		Frozen,
		UnFrozen
	};

	FreezeState freeze_state() const;
 
	virtual void freeze (InterThreadInfo&) = 0;
	virtual void unfreeze () = 0;

	virtual boost::shared_ptr<Region> bounce (InterThreadInfo&) = 0;
	virtual boost::shared_ptr<Region> bounce_range (nframes_t start, nframes_t end, InterThreadInfo&, bool enable_processing = true) = 0;

	XMLNode&    get_state();
	XMLNode&    get_template();
	virtual int set_state(const XMLNode& node) = 0;

	static void zero_diskstream_id_in_xml (XMLNode&);

	PBD::Controllable& rec_enable_control() { return _rec_enable_control; }

	bool record_enabled() const;
	void set_record_enable (bool yn, void *src);
	
	void set_meter_point (MeterPoint, void* src);
	
	sigc::signal<void> DiskstreamChanged;
	sigc::signal<void> FreezeChange;

  protected:
	Track (Session& sess, const XMLNode& node, DataType default_type = DataType::AUDIO);

	virtual XMLNode& state (bool full) = 0;

	virtual void passthru_silence (nframes_t start_frame, nframes_t end_frame, 
				       nframes_t nframes, int declick, bool meter) = 0;

	virtual uint32_t n_process_buffers () = 0;
	
	boost::shared_ptr<Diskstream> _diskstream;
	MeterPoint  _saved_meter_point;
	TrackMode   _mode;

	//private: (FIXME)
	struct FreezeRecordInsertInfo {
	    FreezeRecordInsertInfo(XMLNode& st, boost::shared_ptr<Insert> ins) 
		    : state (st), insert (ins) {}

	    XMLNode                   state;
	    boost::shared_ptr<Insert> insert;
	    PBD::ID                   id;
	    UndoAction                memento;
	};

	struct FreezeRecord {
	    FreezeRecord()
		: have_mementos(false)
	    {}

	    ~FreezeRecord();

	    boost::shared_ptr<Playlist>     playlist;
	    vector<FreezeRecordInsertInfo*> insert_info;
	    bool                            have_mementos;
	    FreezeState                     state;
	    gain_t                          gain;
	    AutoState                       gain_automation_state;
	    AutoState                       pan_automation_state;
	};

	struct RecEnableControllable : public PBD::Controllable {
	    RecEnableControllable (Track&);
	    
	    void set_value (float);
	    float get_value (void) const;

	    Track& track;
	};

	nframes_t check_initial_delay (nframes_t nframes, nframes_t&);

	//virtual void diskstream_record_enable_changed (void *src) = 0;
	//virtual void diskstream_input_channel_changed (void *src) = 0;

	//virtual void input_change_handler (void *src) = 0;

	virtual void set_state_part_two () = 0;

	FreezeRecord          _freeze_record;
	XMLNode*              pending_state;
	sigc::connection      recenable_connection;
	sigc::connection      ic_connection;
	RecEnableControllable _rec_enable_control;
	bool                  _destructive;
};

}; /* namespace ARDOUR*/

#endif /* __ardour_track_h__ */
