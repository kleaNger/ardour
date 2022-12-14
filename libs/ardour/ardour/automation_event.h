/*
    Copyright (C) 2002 Paul Davis 

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

#ifndef __ardour_automation_event_h__
#define __ardour_automation_event_h__

#include <stdint.h>
#include <list>
#include <cmath>

#include <boost/pool/pool.hpp>
#include <boost/pool/pool_alloc.hpp>

#include <sigc++/signal.h>
#include <glibmm/thread.h>

#include <pbd/undo.h>
#include <pbd/xml++.h>
#include <pbd/statefuldestructible.h> 

#include <ardour/ardour.h>

namespace ARDOUR {
	
class ControlEvent {
  public:
    double when;
    double value;
    
    ControlEvent (double w, double v)
	    : when (w), value (v) { }
    ControlEvent (const ControlEvent& other) 
	    : when (other.when), value (other.value) {}

    virtual ~ControlEvent() {}
    
//    bool operator==(const ControlEvent& other) {
//	    return value == other.value && when == other.when;
//    }
};

/* automation lists use a pool allocator that does not use a lock and 
   allocates 8k of new pointers at a time
*/

typedef boost::fast_pool_allocator<ControlEvent*,
	boost::default_user_allocator_new_delete,
	boost::details::pool::null_mutex,
	8192> ControlEventAllocator;

class AutomationList : public PBD::StatefulDestructible
{
  public:
	typedef std::list<ControlEvent*,ControlEventAllocator> AutomationEventList;
	typedef AutomationEventList::iterator iterator;
	typedef AutomationEventList::reverse_iterator reverse_iterator;
	typedef AutomationEventList::const_iterator const_iterator;

	AutomationList (double default_value);
	AutomationList (const XMLNode&);
	~AutomationList();

	AutomationList (const AutomationList&);
	AutomationList (const AutomationList&, double start, double end);
	AutomationList& operator= (const AutomationList&);
	bool operator== (const AutomationList&);

	void freeze();
	void thaw ();
        int8_t frozen() const { return _frozen; }

	AutomationEventList::size_type size() const { return events.size(); }
	bool empty() const { return events.empty(); }

	void reset_default (double val) {
		default_value = val;
	}

	void clear ();
	void x_scale (double factor);
	bool extend_to (double);
	void slide (iterator before, double distance);
	
	void write_pass_finished (double when);
	void rt_add (double when, double value);
	void add (double when, double value);
	/* this should be private but old-school automation loading needs it in IO/Redirect */
	void fast_simple_add (double when, double value);
        void merge_nascent (double when);

	void reset_range (double start, double end);
	void erase_range (double start, double end);
	void erase (iterator);
	void erase (iterator, iterator);
	void move_range (iterator start, iterator end, double, double);
	void modify (iterator, double, double);
	void shift (nframes64_t, nframes64_t);

	AutomationList* cut (double, double);
	AutomationList* copy (double, double);
	void clear (double, double);

	bool paste (AutomationList&, double position, float times);

	void set_automation_state (AutoState);
	AutoState automation_state() const { return _state; }
	sigc::signal<void> automation_style_changed;

	void set_automation_style (AutoStyle m);
        AutoStyle automation_style() const { return _style; }
	sigc::signal<void> automation_state_changed;

	bool automation_playback() {
		return (_state & Auto_Play) || ((_state & Auto_Touch) && !touching());
	}
	bool automation_write () {
		return (_state & Auto_Write) || ((_state & Auto_Touch) && touching());
	}

	void start_touch (double when);
	void stop_touch (bool mark, double when, double value);
        bool touching() const { return g_atomic_int_get (&_touching); }

	void set_yrange (double min, double max) {
		min_yval = min;
		max_yval = max;
	}

	double get_max_y() const { return max_yval; }
	double get_min_y() const { return min_yval; }

	void truncate_end (double length);
	void truncate_start (double length);
	
	iterator begin() { return events.begin(); }
	iterator end() { return events.end(); }

	ControlEvent* back() { return events.back(); }
	ControlEvent* front() { return events.front(); }

	const_iterator const_begin() const { return events.begin(); }
	const_iterator const_end() const { return events.end(); }

	std::pair<AutomationList::iterator,AutomationList::iterator> control_points_adjacent (double when);

	template<class T> void apply_to_points (T& obj, void (T::*method)(const AutomationList&)) {
		Glib::Mutex::Lock lm (lock);
		(obj.*method)(*this);
	}

	sigc::signal<void> StateChanged;

	XMLNode& get_state(void); 
	int set_state (const XMLNode &s);
	XMLNode& state (bool full);
	XMLNode& serialize_events ();

	void set_max_xval (double);
	double get_max_xval() const { return max_xval; }

	double eval (double where) {
		Glib::Mutex::Lock lm (lock);
		return unlocked_eval (where);
	}

	double rt_safe_eval (double where, bool& ok) {

		Glib::Mutex::Lock lm (lock, Glib::TRY_LOCK);

		if ((ok = lm.locked())) {
			return unlocked_eval (where);
		} else {
			return 0.0;
		}
	}

	struct TimeComparator {
		bool operator() (const ControlEvent* a, const ControlEvent* b) { 
			return a->when < b->when;
		}
	};

        static sigc::signal<void, AutomationList*> AutomationListCreated;

  protected:
        
        struct NascentInfo {
            AutomationEventList events;
            bool   is_touch;
            double start_time;
            double end_time;
            
            NascentInfo (bool touching, double start = -1.0)
                    : is_touch (touching)
                    , start_time (start)
                    , end_time (-1.0) 
            {}
        };

	AutomationEventList events;
        std::list<NascentInfo*> nascent;
	mutable Glib::Mutex lock;
	int8_t  _frozen;
	bool    changed_when_thawed;
	bool   _dirty;

	struct LookupCache {
	    double left;  /* leftmost x coordinate used when finding "range" */
	    std::pair<AutomationList::iterator,AutomationList::iterator> range;
	};

	LookupCache lookup_cache;

	AutoState  _state;
	AutoStyle  _style;
        gint       _touching;
	bool       _new_touch;
	double      max_xval;
	double      min_yval;
	double      max_yval;
	double      default_value;
	bool        sort_pending;

	void maybe_signal_changed ();
	void mark_dirty ();
	void _x_scale (double factor);

	/* called by type-specific unlocked_eval() to handle
	   common case of 0, 1 or 2 control points.
	*/

	double shared_eval (double x);

	/* called by shared_eval() to handle any case of
	   3 or more control points.
	*/

	virtual double multipoint_eval (double x); 

	/* called by locked entry point and various private
	   locations where we already hold the lock.
	*/

	virtual double unlocked_eval (double where);

	virtual ControlEvent* point_factory (double,double) const;
	virtual ControlEvent* point_factory (const ControlEvent&) const;

	AutomationList* cut_copy_clear (double, double, int op);

	int deserialize_events (const XMLNode&);
};

} // namespace

#endif /* __ardour_automation_event_h__ */
