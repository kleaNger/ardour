/*
	Copyright (C) 2006,2007 John Anderson

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
#ifndef route_signal_h
#define route_signal_h

#include <sigc++/sigc++.h>
#include <boost/shared_ptr.hpp>

#include <vector>

#include "midi_byte_array.h"

class MackieControlProtocol;

namespace ARDOUR {
	class Route;
}
	
namespace Mackie
{

class Strip;
class SurfacePort;

/**
  This class is intended to easily create and destroy the set of
  connections from a route to a control surface strip. Instantiating
  it will connect the signals, and destructing it will disconnect
  the signals.
*/
class RouteSignal
{
public:
        RouteSignal(boost::shared_ptr<ARDOUR::Route> route, MackieControlProtocol & mcp, Strip & strip, SurfacePort & port )
	: _route( route ), _mcp( mcp ), _strip( strip ), _port( port ), _last_gain_written(0.0)
	{
		connect();
	}
	
	~RouteSignal()
	{
		disconnect();
	}
	
	void connect();
	void disconnect();
	
	// call all signal handlers manually
	void notify_all();
	
	boost::shared_ptr<const ARDOUR::Route> route() const { return _route; }
	Strip & strip() { return _strip; }
	SurfacePort & port() { return _port; }
	
	float last_gain_written() const { return _last_gain_written; }
	void last_gain_written( float other ) { _last_gain_written = other; }
	
	const MidiByteArray & last_pan_written() const { return _last_pan_written; }
	void last_pan_written( const MidiByteArray & other ) { _last_pan_written = other; }
	
private:
	boost::shared_ptr<ARDOUR::Route> _route;
	MackieControlProtocol & _mcp;
	Strip & _strip;
	SurfacePort & _port;

	typedef std::vector<sigc::connection> Connections;
	Connections _connections;

	// Last written values for the gain and pan, to avoid overloading
	// the midi connection to the surface
	float _last_gain_written;
	MidiByteArray _last_pan_written;
};

}

#endif
