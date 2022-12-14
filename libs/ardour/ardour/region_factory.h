/*
    Copyright (C) 2000-2007 Paul Davis 

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

#ifndef __ardour_region_factory_h__
#define __ardour_region_factory_h__

#include <ardour/types.h>
#include <ardour/region.h>

class XMLNode;

namespace ARDOUR {

class Session;

class RegionFactory {

  public:
	/* This is emitted only when a new id is assigned. Therefore,
	   in a pure Region copy, it will not be emitted.

	   It must be emitted by derived classes, not Region
	   itself, to permit dynamic_cast<> to be used to 
	   infer the type of Region.
	*/

	static sigc::signal<void,boost::shared_ptr<Region> > CheckNewRegion;

	static boost::shared_ptr<Region> create (boost::shared_ptr<const Region>);

	/* note: both of the first two should use const shared_ptr as well, but
	   gcc 4.1 doesn't seem to be able to disambiguate them if they do.
	*/

	static boost::shared_ptr<Region> create (boost::shared_ptr<Region>, nframes_t start, 
						 nframes_t length, const std::string& name, 
						 layer_t = 0, Region::Flag flags = Region::DefaultFlags, bool announce = true);
	static boost::shared_ptr<Region> create (boost::shared_ptr<AudioRegion>, nframes_t start, 
						 nframes_t length, const std::string& name, 
						 layer_t = 0, Region::Flag flags = Region::DefaultFlags, bool announce = true);
	static boost::shared_ptr<Region> create (boost::shared_ptr<Region>, const SourceList&, const string& name, layer_t = 0, Region::Flag flags = Region::DefaultFlags, bool announce = true);
	static boost::shared_ptr<Region> create (boost::shared_ptr<Source>, nframes_t start, nframes_t length, const string& name, layer_t = 0, Region::Flag flags = Region::DefaultFlags, bool announce = true);
	static boost::shared_ptr<Region> create (const SourceList &, nframes_t start, nframes_t length, const string& name, layer_t = 0, Region::Flag flags = Region::DefaultFlags, bool announce = true);
	static boost::shared_ptr<Region> create (Session&, XMLNode&, bool);
	static boost::shared_ptr<Region> create (SourceList &, const XMLNode&);
};

}

#endif /* __ardour_region_factory_h__  */
