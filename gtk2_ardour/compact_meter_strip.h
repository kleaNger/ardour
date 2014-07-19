/*
    Copyright (C) 2014 Waves Audio Ltd.

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

#ifndef __tracks_compact_meter_strip__
#define __tracks_compact_meter_strip__

#include "waves_ui.h"
#include "level_meter.h"

namespace ARDOUR {
	class Route;
	class Session;
}

class CompactMeterStrip :public Gtk::EventBox, public WavesUI
{
  public:
	CompactMeterStrip (ARDOUR::Session*, boost::shared_ptr<ARDOUR::Route>);
	~CompactMeterStrip ();

	void fast_update ();
	boost::shared_ptr<ARDOUR::Route> route() { return _route; }
	static PBD::Signal1<void,CompactMeterStrip*> CatchDeletion;


  protected:
	boost::shared_ptr<ARDOUR::Route> _route;
	void self_delete ();

  private:
	Gtk::Box&       level_meter_home;
	LevelMeterHBox level_meter;
	PBD::ScopedConnectionList route_connections;
	static int __meter_width;
	void meter_configuration_changed (ARDOUR::ChanCount);
	void update_rec_display ();
};

#endif /* __tracks_compact_meter_strip__ */
