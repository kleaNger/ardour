
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

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cfloat>
#include <unistd.h>

#include <pbd/id.h>

#include <ardour/curve.h>

using namespace std;
using namespace ARDOUR;
using namespace PBD;

int
curvetest (string filename)
{
	// needed to initialize ID objects/counter used
	// by Curve et al.
	PBD::ID::init ();

	ifstream in (filename.c_str());
	stringstream line;
	Curve c (-1.0, +1.0, 0, true);
	double minx = DBL_MAX;
	double maxx = DBL_MIN;

	while (in) {
		double x, y;

		in >> x;
		in >> y;
		
		if (!in) {
			break;
		}

		if (x < minx) {
			minx = x;
		}
		
		if (x > maxx) {
			maxx = x;
		}
		
		c.add (x, y);
	}


	float foo[1024];

	c.get_vector (minx, maxx, foo, 1024);
	
	for (int i = 0; i < 1024; ++i) {
	        cout << setw(20) << setprecision(20) << minx + (((double) i / 1024.0) * (maxx - minx)) << ' ' << foo[i] << endl;
	}
	
	return 0;
}
