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

#ifndef __ardour_insert_h__
#define __ardour_insert_h__

#include <vector>
#include <string>
#include <exception>

#include <sigc++/signal.h>
#include <ardour/ardour.h>
#include <ardour/redirect.h>
#include <ardour/types.h>

class XMLNode;
class MTDM;

namespace MIDI {
	class Port;
}

namespace ARDOUR {

class Session;
class Route;
class Plugin;

class Insert : public Redirect
{
  public:
	Insert(Session& s, std::string name, Placement p);
	Insert(Session& s, std::string name, Placement p, int imin, int imax, int omin, int omax);
	
	virtual ~Insert() { }

	virtual void run (vector<Sample *>& bufs, uint32_t nbufs, nframes_t nframes) = 0;
	virtual void activate () {}
	virtual void deactivate () {}
	virtual void flush () {}

	virtual int32_t can_do (int32_t in, int32_t& out) = 0;
	virtual int32_t configure_io (int32_t magic, int32_t in, int32_t out) = 0;
};

class PortInsert : public Insert 
{
  public:
	PortInsert (Session&, Placement);
	PortInsert (Session&, const XMLNode&);
	PortInsert (const PortInsert&);
	~PortInsert ();

	XMLNode& state(bool full);
	XMLNode& get_state(void);
	int set_state(const XMLNode&);

	void init ();
	void run (vector<Sample *>& bufs, uint32_t nbufs, nframes_t nframes);

	nframes_t latency();
	
	uint32_t output_streams() const;
	uint32_t input_streams() const;

	int32_t can_do (int32_t, int32_t& out);
	int32_t configure_io (int32_t magic, int32_t in, int32_t out);
	uint32_t bit_slot() const { return bitslot; }

	void start_latency_detection ();
	void stop_latency_detection ();

	MTDM* mtdm () const { return _mtdm; }
	void set_measured_latency (nframes_t);

  private:
	uint32_t   bitslot;
	MTDM*     _mtdm;
	bool      _latency_detect;
	nframes_t _latency_flush_frames;
	nframes_t _measured_latency;
};

class PluginInsert : public Insert
{
  public:
	PluginInsert (Session&, boost::shared_ptr<Plugin>, Placement);
	PluginInsert (Session&, const XMLNode&);
	PluginInsert (const PluginInsert&);
	~PluginInsert ();

	static const string port_automation_node_name;
	
	XMLNode& state(bool);
	XMLNode& get_state(void);
	int set_state(const XMLNode&);

	void run (vector<Sample *>& bufs, uint32_t nbufs, nframes_t nframes);
	void silence (nframes_t nframes);
	void activate ();
	void deactivate ();
	void flush ();

	int set_block_size (nframes_t nframes);

	uint32_t output_streams() const;
	uint32_t input_streams() const;
	uint32_t natural_output_streams() const;
	uint32_t natural_input_streams() const;

	int      set_count (uint32_t num);
	uint32_t get_count () const { return _plugins.size(); }

	int32_t can_do (int32_t, int32_t& out);
	int32_t configure_io (int32_t magic, int32_t in, int32_t out);

	bool is_generator() const;

	void set_parameter (uint32_t port, float val);

	AutoState get_port_automation_state (uint32_t port);
	void set_port_automation_state (uint32_t port, AutoState);
	void protect_automation ();

	float default_parameter_value (uint32_t which);

	boost::shared_ptr<Plugin> plugin(uint32_t num=0) const {
		if (num < _plugins.size()) { 
			return _plugins[num];
		} else {
			return _plugins[0]; // we always have one
		}
	}

	PluginType type ();

	string describe_parameter (uint32_t);

	nframes_t latency();

	void transport_stopped (nframes_t now);
	void automation_snapshot (nframes_t now, bool force);

  private:

	void parameter_changed (uint32_t, float);
	
	vector<boost::shared_ptr<Plugin> > _plugins;
	void automation_run (vector<Sample *>& bufs, uint32_t nbufs, nframes_t nframes);
	void connect_and_run (vector<Sample *>& bufs, uint32_t nbufs, nframes_t nframes, nframes_t offset, bool with_auto, nframes_t now = 0);

	void init ();
	void set_automatable ();
	void auto_state_changed (uint32_t which);
	void automation_list_creation_callback (uint32_t, AutomationList&);

	boost::shared_ptr<Plugin> plugin_factory (boost::shared_ptr<Plugin>);
};

} // namespace ARDOUR

#endif /* __ardour_insert_h__ */
