/*
    Copyright (C) 2004 Paul Davis 

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

#include <cmath>
#include <cerrno>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <locale.h>
#include <unistd.h>
#include <float.h>

#include <glibmm.h>

#include <pbd/error.h>
#include <pbd/failed_constructor.h>
#include <pbd/xml++.h>
#include <pbd/enumwriter.h>
#include <pbd/localeguard.h>

#include <ardour/session.h>
#include <ardour/panner.h>
#include <ardour/utils.h>

#include <ardour/mix.h>

#include "i18n.h"

#include <pbd/mathfix.h>

using namespace std;
using namespace ARDOUR;
using namespace PBD;

float Panner::current_automation_version_number = 1.0;

string EqualPowerStereoPanner::name = "Equal Power Stereo";
string Multi2dPanner::name = "Multiple (2D)";

/* this is a default mapper of  control values to a pan position
   others can be imagined. 
*/

static pan_t direct_control_to_pan (double fract) { 
	return fract;
}

static double direct_pan_to_control (pan_t val) { 
	return val;
}

StreamPanner::StreamPanner (Panner& p)
	: parent (p),
	  _control (X_("panner"), *this)
{
	_muted = false;

	parent.session().add_controllable (&_control);

	x = 0.5;
	y = 0.5;
	z = 0.5;
}

StreamPanner::~StreamPanner ()
{
}

void
StreamPanner::PanControllable::set_value (float val)
{
	panner.set_position (direct_control_to_pan (val));
}

float
StreamPanner::PanControllable::get_value (void) const
{
	float xpos;
	panner.get_effective_position (xpos);
	return direct_pan_to_control (xpos);
}

bool
StreamPanner::PanControllable::can_send_feedback () const
{
	AutoState astate = panner.get_parent().automation_state ();

	if ((astate == Auto_Play) || (astate == Auto_Touch && !panner.get_parent().touching())) {
		return true;
	}

	return false;
}

void
StreamPanner::set_muted (bool yn)
{
	if (yn != _muted) {
		_muted = yn;
		StateChanged ();
	}
}

void
StreamPanner::set_position (float xpos, bool link_call)
{
	if (!link_call && parent.linked()) {
		parent.set_position (xpos, *this);
	}

	if (x != xpos) {
		x = xpos;
		update ();
		Changed ();
		_control.Changed ();
	}
}

void
StreamPanner::set_position (float xpos, float ypos, bool link_call)
{
	if (!link_call && parent.linked()) {
		parent.set_position (xpos, ypos, *this);
	}

	if (x != xpos || y != ypos) {
		
		x = xpos;
		y = ypos;
		update ();
		Changed ();
	}
}

void
StreamPanner::set_position (float xpos, float ypos, float zpos, bool link_call)
{
	if (!link_call && parent.linked()) {
		parent.set_position (xpos, ypos, zpos, *this);
	}

	if (x != xpos || y != ypos || z != zpos) {
		x = xpos;
		y = ypos;
		z = zpos;
		update ();
		Changed ();
	}
}

int
StreamPanner::set_state (const XMLNode& node)
{
	const XMLProperty* prop;
	XMLNodeConstIterator iter;

	if ((prop = node.property (X_("muted")))) {
		set_muted (string_is_affirmative (prop->value()));
	}

	return 0;
}

void
StreamPanner::add_state (XMLNode& node)
{
	node.add_property (X_("muted"), (muted() ? "yes" : "no"));
}

/*---------------------------------------------------------------------- */

BaseStereoPanner::BaseStereoPanner (Panner& p)
	: StreamPanner (p), _automation (0.0, 1.0, 0.5)
{
}

BaseStereoPanner::~BaseStereoPanner ()
{
}

void
BaseStereoPanner::snapshot (nframes_t now)
{
	if (_automation.automation_write () && parent.session().transport_rolling()) {
		_automation.rt_add (now, x);
	}
}

void
BaseStereoPanner::transport_stopped (nframes_t frame)
{
	if (_automation.automation_state() == Auto_Touch || _automation.automation_state() == Auto_Play) {
		set_position (_automation.eval (frame));
	}

	_automation.write_pass_finished (frame);
}

void
BaseStereoPanner::set_automation_style (AutoStyle style)
{
	_automation.set_automation_style (style);
}

void
BaseStereoPanner::set_automation_state (AutoState state)
{
	if (state != _automation.automation_state()) {

		_automation.set_automation_state (state);

		/* don't reset pan if we're moving to Off or Write mode;
		   if we're moving to Write, the user may have manually set up pans
		   that they don't want to lose */		
		if (state != Auto_Off && state != Auto_Write) {
			set_position (_automation.eval (parent.session().transport_frame()));
		}
	}
}

int
BaseStereoPanner::load (istream& in, string path, uint32_t& linecnt)
{
	char line[128];
	LocaleGuard lg (X_("POSIX"));
	
	_automation.clear ();

	while (in.getline (line, sizeof (line), '\n')) {
		jack_nframes_t when;
		double value;

		++linecnt;

		if (strcmp (line, "end") == 0) {
			break;
		}

		if (sscanf (line, "%" PRIu32 " %lf", &when, &value) != 2) {
			warning << string_compose(_("badly formatted pan automation event record at line %1 of %2 (ignored) [%3]"), linecnt, path, line) << endmsg;
			continue;
		}

		_automation.fast_simple_add (when, value);
	}

	/* now that we are done loading */

	_automation.StateChanged ();

	return 0;
}

void
BaseStereoPanner::distribute (Sample* src, Sample** obufs, gain_t gain_coeff, nframes_t nframes)
{
	pan_t delta;
	Sample* dst;
	pan_t pan;

	if (_muted) {
		return;
	}

	/* LEFT */

	dst = obufs[0];

	if (fabsf ((delta = (left - desired_left))) > 0.002) { // about 1 degree of arc 
		
		/* interpolate over 64 frames or nframes, whichever is smaller */
		
		nframes_t limit = min ((nframes_t)64, nframes);
		nframes_t n;

		delta = -(delta / (float) (limit));
		
		for (n = 0; n < limit; n++) {
			left_interp = left_interp + delta;
			left = left_interp + 0.9 * (left - left_interp);
			dst[n] += src[n] * left * gain_coeff;
		}
		
		pan = left * gain_coeff;

		Session::mix_buffers_with_gain(dst+n,src+n,nframes-n,pan);
		
	} else {
		
		left = desired_left;
		left_interp = left;

		if ((pan = (left * gain_coeff)) != 1.0f) {
			
			if (pan != 0.0f) {
				
				Session::mix_buffers_with_gain(dst,src,nframes,pan);

				/* mark that we wrote into the buffer */

				// obufs[0] = 0;

			} 
			
		} else {
			
			Session::mix_buffers_no_gain(dst,src,nframes);
			
			/* mark that we wrote into the buffer */
			
			// obufs[0] = 0;
		}
	}

	/* RIGHT */

	dst = obufs[1];
	
	if (fabsf ((delta = (right - desired_right))) > 0.002) { // about 1 degree of arc 
		
		/* interpolate over 64 frames or nframes, whichever is smaller */
		
		nframes_t limit = min ((nframes_t)64, nframes);
		nframes_t n;

		delta = -(delta / (float) (limit));

		for (n = 0; n < limit; n++) {
			right_interp = right_interp + delta;
			right = right_interp + 0.9 * (right - right_interp);
			dst[n] += src[n] * right * gain_coeff;
		}
		
		pan = right * gain_coeff;
		
		Session::mix_buffers_with_gain(dst+n,src+n,nframes-n,pan);
		
		/* XXX it would be nice to mark the buffer as written to */

	} else {

		right = desired_right;
		right_interp = right;
		
		if ((pan = (right * gain_coeff)) != 1.0f) {
			
			if (pan != 0.0f) {
				
				Session::mix_buffers_with_gain(dst,src,nframes,pan);
				
				/* XXX it would be nice to mark the buffer as written to */
			}
			
		} else {
			
			Session::mix_buffers_no_gain(dst,src,nframes);
			
			/* XXX it would be nice to mark the buffer as written to */
		}
	}
}

/*---------------------------------------------------------------------- */

EqualPowerStereoPanner::EqualPowerStereoPanner (Panner& p)
	: BaseStereoPanner (p)
{
	update ();

	left = desired_left;
	right = desired_right;
	left_interp = left;
	right_interp = right;
}

EqualPowerStereoPanner::~EqualPowerStereoPanner ()
{
}

void
EqualPowerStereoPanner::update ()
{
	/* it would be very nice to split this out into a virtual function
	   that can be accessed from BaseStereoPanner and used in distribute_automated().
	   
	   but the place where its used in distribute_automated() is a tight inner loop,
	   and making "nframes" virtual function calls to compute values is an absurd
	   overhead.
	*/

	/* x == 0 => hard left
	   x == 1 => hard right
	*/

	float panR = x;
	float panL = 1 - panR;

	const float pan_law_attenuation = -3.0f;
	const float scale = 2.0f - 4.0f * powf (10.0f,pan_law_attenuation/20.0f);
	
	desired_left = panL * (scale * panL + 1.0f - scale);
	desired_right = panR * (scale * panR + 1.0f - scale);

	effective_x = x;
}

void
EqualPowerStereoPanner::distribute_automated (Sample* src, Sample** obufs, 
					      nframes_t start, nframes_t end, nframes_t nframes,
					      pan_t** buffers)
{
	Sample* dst;
	pan_t* pbuf;

	/* fetch positional data */

	if (!_automation.rt_safe_get_vector (start, end, buffers[0], nframes)) {
		/* fallback */
		if (!_muted) {
			distribute (src, obufs, 1.0, nframes);
		}
		return;
	}

	/* store effective pan position. do this even if we are muted */

	if (nframes > 0) 
		effective_x = buffers[0][nframes-1];

	if (_muted) {
		return;
	}

	/* apply pan law to convert positional data into pan coefficients for
	   each buffer (output)
	*/

	const float pan_law_attenuation = -3.0f;
	const float scale = 2.0f - 4.0f * powf (10.0f,pan_law_attenuation/20.0f);

	for (nframes_t n = 0; n < nframes; ++n) {

		float panR = buffers[0][n];
		float panL = 1 - panR;
		
		buffers[0][n] = panL * (scale * panL + 1.0f - scale);
		buffers[1][n] = panR * (scale * panR + 1.0f - scale);
	}

	/* LEFT */

	dst = obufs[0];
	pbuf = buffers[0];
	
	for (nframes_t n = 0; n < nframes; ++n) {
		dst[n] += src[n] * pbuf[n];
	}	

        /* XXX it would be nice to mark the buffer as written to */

	/* RIGHT */

	dst = obufs[1];
	pbuf = buffers[1];

	for (nframes_t n = 0; n < nframes; ++n) {
		dst[n] += src[n] * pbuf[n];
	}	
	
        /* XXX it would be nice to mark the buffer as written to */
}

StreamPanner*
EqualPowerStereoPanner::factory (Panner& parent)
{
	return new EqualPowerStereoPanner (parent);
}

XMLNode&
EqualPowerStereoPanner::get_state (void)
{
	return state (true);
}

XMLNode&
EqualPowerStereoPanner::state (bool full_state)
{
	XMLNode* root = new XMLNode ("StreamPanner");
	char buf[64];
	LocaleGuard lg (X_("POSIX"));

	snprintf (buf, sizeof (buf), "%.12g", x); 
	root->add_property (X_("x"), buf);
	root->add_property (X_("type"), EqualPowerStereoPanner::name);

	XMLNode* autonode = new XMLNode (X_("Automation"));
	autonode->add_child_nocopy (_automation.state (full_state));
	root->add_child_nocopy (*autonode);

	StreamPanner::add_state (*root);

	root->add_child_nocopy (_control.get_state ());

	return *root;
}

int
EqualPowerStereoPanner::set_state (const XMLNode& node)
{
	const XMLProperty* prop;
	float pos;
	LocaleGuard lg (X_("POSIX"));

	if ((prop = node.property (X_("x")))) {
		pos = atof (prop->value().c_str());
		set_position (pos, true);
	} 

	StreamPanner::set_state (node);

	for (XMLNodeConstIterator iter = node.children().begin(); iter != node.children().end(); ++iter) {

		if ((*iter)->name() == X_("controllable")) {
			if ((prop = (*iter)->property("name")) != 0 && prop->value() == "panner") {
				_control.set_state (**iter);
			}

		} else if ((*iter)->name() == X_("Automation")) {

			_automation.set_state (*((*iter)->children().front()));

			if (_automation.automation_state() != Auto_Off) {
				set_position (_automation.eval (parent.session().transport_frame()));
			}
		}
	}

	return 0;
}

/*----------------------------------------------------------------------*/

Multi2dPanner::Multi2dPanner (Panner& p)
	: StreamPanner (p), _automation (0.0, 1.0, 0.5) // XXX useless
{
	update ();
}

Multi2dPanner::~Multi2dPanner ()
{
}

void
Multi2dPanner::snapshot (nframes_t now)
{
	// how?
}

void
Multi2dPanner::transport_stopped (nframes_t frame)
{
	//what?
}

void
Multi2dPanner::set_automation_style (AutoStyle style)
{
	//what?
}

void
Multi2dPanner::set_automation_state (AutoState state)
{
	// what?
}

void
Multi2dPanner::update ()
{
	static const float BIAS = FLT_MIN;
	uint32_t i;
	uint32_t nouts = parent.outputs.size();
	float dsq[nouts];
	float f, fr;
	vector<pan_t> pans;

	f = 0.0f;

	for (i = 0; i < nouts; i++) {
		dsq[i] = ((x - parent.outputs[i].x) * (x - parent.outputs[i].x) + (y - parent.outputs[i].y) * (y - parent.outputs[i].y) + BIAS);
		if (dsq[i] < 0.0) {
			dsq[i] = 0.0;
		}
		f += dsq[i] * dsq[i];
	}
#ifdef __APPLE__
	// terrible hack to support OSX < 10.3.9 builds
	fr = (float) (1.0 / sqrt((double)f));
#else
	fr = 1.0 / sqrtf(f);
#endif	
	for (i = 0; i < nouts; ++i) {
		parent.outputs[i].desired_pan = 1.0f - (dsq[i] * fr);
	}

	effective_x = x;
}

void
Multi2dPanner::distribute (Sample* src, Sample** obufs, gain_t gain_coeff, nframes_t nframes)
{
	Sample* dst;
	pan_t pan;
	vector<Panner::Output>::iterator o;
	uint32_t n;

	if (_muted) {
		return;
	}


	for (n = 0, o = parent.outputs.begin(); o != parent.outputs.end(); ++o, ++n) {

		dst = obufs[n];
	
#ifdef CAN_INTERP
		if (fabsf ((delta = (left_interp - desired_left))) > 0.002) { // about 1 degree of arc 
			
			/* interpolate over 64 frames or nframes, whichever is smaller */
			
			nframes_t limit = min ((nframes_t)64, nframes);
			nframes_t n;
			
			delta = -(delta / (float) (limit));
		
			for (n = 0; n < limit; n++) {
				left_interp = left_interp + delta;
				left = left_interp + 0.9 * (left - left_interp);
				dst[n] += src[n] * left * gain_coeff;
			}
			
			pan = left * gain_coeff;
			Session::mix_buffers_with_gain(dst+n,src+n,nframes-n,pan);
			
		} else {

#else			
			pan = (*o).desired_pan;
			
			if ((pan *= gain_coeff) != 1.0f) {
				
				if (pan != 0.0f) {
					Session::mix_buffers_with_gain(dst,src,nframes,pan);
				} 
			} else {
					Session::mix_buffers_no_gain(dst,src,nframes);
			}
#endif
#ifdef CAN_INTERP
		}
#endif
	}
	
	return;
}

void
Multi2dPanner::distribute_automated (Sample* src, Sample** obufs, 
				     nframes_t start, nframes_t end, nframes_t nframes,
				     pan_t** buffers)
{
	if (_muted) {
		return;
	}

	/* what ? */

	return;
}

StreamPanner*
Multi2dPanner::factory (Panner& p)
{
	return new Multi2dPanner (p);
}

int
Multi2dPanner::load (istream& in, string path, uint32_t& linecnt)
{
	return 0;
}

XMLNode&
Multi2dPanner::get_state (void)
{
	return state (true);
}

XMLNode&
Multi2dPanner::state (bool full_state)
{
	XMLNode* root = new XMLNode ("StreamPanner");
	char buf[64];
	LocaleGuard lg (X_("POSIX"));

	snprintf (buf, sizeof (buf), "%.12g", x); 
	root->add_property (X_("x"), buf);
	snprintf (buf, sizeof (buf), "%.12g", y); 
	root->add_property (X_("y"), buf);
	root->add_property (X_("type"), Multi2dPanner::name);

	/* XXX no meaningful automation yet */

	return *root;
}

int
Multi2dPanner::set_state (const XMLNode& node)
{
	const XMLProperty* prop;
	float newx,newy;
	LocaleGuard lg (X_("POSIX"));

	newx = -1;
	newy = -1;

	if ((prop = node.property (X_("x")))) {
		newx = atof (prop->value().c_str());
	}
       
	if ((prop = node.property (X_("y")))) {
		newy = atof (prop->value().c_str());
	}
	
	if (x < 0 || y < 0) {
		error << _("badly-formed positional data for Multi2dPanner - ignored")
		      << endmsg;
		return -1;
	} 
	
	set_position (newx, newy);
	return 0;
}

/*---------------------------------------------------------------------- */

Panner::Panner (string name, Session& s)
	: _session (s)
{
	set_name (name);

	_linked = false;
	_link_direction = SameDirection;
	_bypassed = false;
}

Panner::~Panner ()
{
}

void
Panner::set_linked (bool yn)
{
	if (yn != _linked) {
		_linked = yn;
		_session.set_dirty ();
		LinkStateChanged (); /* EMIT SIGNAL */
	}
}

void
Panner::set_link_direction (LinkDirection ld)
{
	if (ld != _link_direction) {
		_link_direction = ld;
		_session.set_dirty ();
		LinkStateChanged (); /* EMIT SIGNAL */
	}
}

void
Panner::set_bypassed (bool yn)
{
	if (yn != _bypassed) {
		_bypassed = yn;
		StateChanged ();
	}
}

void
Panner::reset_to_default ()
{
	vector<float> positions;

	switch (outputs.size()) {
	case 0:
	case 1:
		return;
	}

	if (outputs.size() == 2) {
		switch (size()) {
		case 1:
			front()->set_position (0.5);
			front()->automation().reset_default (0.5);
			return;
			break;
		case 2:
			front()->set_position (0.0);
			front()->automation().reset_default (0.0);
			back()->set_position (1.0);
			back()->automation().reset_default (1.0);
			return;
		default:
			break;
		}
	}
	
	vector<Output>::iterator o;
	iterator p;

	for (o = outputs.begin(), p = begin(); o != outputs.end() && p != end(); ++o, ++p) {
		(*p)->set_position ((*o).x, (*o).y);
	}
}

void
Panner::reset_streampanner (uint32_t which)
{
	if (which >= size() || which >= outputs.size()) {
		return;
	}
	
	switch (outputs.size()) {
	case 0:
	case 1:
		return;

	case 2:
		switch (size()) {
		case 1:
			/* stereo out, 1 stream, default = middle */
			front()->set_position (0.5);
			front()->automation().reset_default (0.5);
			break;
		case 2:
			/* stereo out, 2 streams, default = hard left/right */
			if (which == 0) {
				front()->set_position (0.0);
				front()->automation().reset_default (0.0);
			} else {
				back()->set_position (1.0);
				back()->automation().reset_default (1.0);
			}
			break;
		}
		return;

	default:
		(*this)[which]->set_position (outputs[which].x, outputs[which].y);
	}
}

void
Panner::reset (uint32_t nouts, uint32_t npans)
{
	uint32_t n;
	bool changed = false;
	bool do_not_and_did_not_need_panning = ((nouts < 2) && (outputs.size() < 2));

	/* if new and old config don't need panning, or if 
	   the config hasn't changed, we're done.
	*/

	if (do_not_and_did_not_need_panning || 
	    ((nouts == outputs.size()) && (npans == size()))) {
		return;
	} 

	n = size();
	clear ();

	if (n != npans) {
		changed = true;
	}

	n = outputs.size();
	outputs.clear ();

	if (n != nouts) {
		changed = true;
	}

	if (nouts < 2) {
		goto send_changed;
	}

	switch (nouts) {
	case 0:
		break;

	case 1:
		fatal << _("programming error:")
		      << X_("Panner::reset() called with a single output")
		      << endmsg;
		/*NOTREACHED*/
		break;

	case 2:
		/* line */
		outputs.push_back (Output (0, 0));
		outputs.push_back (Output (1.0, 0));

		for (n = 0; n < npans; ++n) {
			push_back (new EqualPowerStereoPanner (*this));
		}
		break;

	case 3: // triangle
		outputs.push_back (Output  (0.5, 0));
		outputs.push_back (Output  (0, 1.0));
		outputs.push_back (Output  (1.0, 1.0));

		for (n = 0; n < npans; ++n) {
			push_back (new Multi2dPanner (*this));
		}

		break; 

	case 4: // square
		outputs.push_back (Output  (0, 0));
		outputs.push_back (Output  (1.0, 0));
		outputs.push_back (Output  (1.0, 1.0));
		outputs.push_back (Output  (0, 1.0));

		for (n = 0; n < npans; ++n) {
			push_back (new Multi2dPanner (*this));
		}

		break;	

	case 5: //square+offcenter center
		outputs.push_back (Output  (0, 0));
		outputs.push_back (Output  (1.0, 0));
		outputs.push_back (Output  (1.0, 1.0));
		outputs.push_back (Output  (0, 1.0));
		outputs.push_back (Output  (0.5, 0.75));

		for (n = 0; n < npans; ++n) {
			push_back (new Multi2dPanner (*this));
		}

		break;

	default:
		/* XXX horrible placement. FIXME */
		for (n = 0; n < nouts; ++n) {
			outputs.push_back (Output (0.1 * n, 0.1 * n));
		}

		for (n = 0; n < npans; ++n) {
			push_back (new Multi2dPanner (*this));
		}

		break;
	}

	for (iterator x = begin(); x != end(); ++x) {
		(*x)->update ();
	}

	/* force hard left/right panning in a common case: 2in/2out 
	*/
	
	if (npans == 2 && outputs.size() == 2) {

		/* Do this only if we changed configuration, or our configuration
		   appears to be the default set up (center).
		*/

		float left;
		float right;

		front()->get_position (left);
		back()->get_position (right);

		if (changed || ((left == 0.5) && (right == 0.5))) {
		
			front()->set_position (0.0);
			front()->automation().reset_default (0.0);
			
			back()->set_position (1.0);
			back()->automation().reset_default (1.0);
			
			changed = true;
		}
	}

  send_changed:
	if (changed) {
		Changed (); /* EMIT SIGNAL */
	}

	return;
}

void
Panner::remove (uint32_t which)
{
	vector<StreamPanner*>::iterator i;
	for (i = begin(); i != end() && which; ++i, --which);

	if (i != end()) {
		delete *i;
		erase (i);
	}
}

void
Panner::clear ()
{
	for (vector<StreamPanner*>::iterator i = begin(); i != end(); ++i) {
		delete *i;
	}

	vector<StreamPanner*>::clear ();
}

void
Panner::set_automation_style (AutoStyle style)
{
	for (vector<StreamPanner*>::iterator i = begin(); i != end(); ++i) {
		(*i)->set_automation_style (style);
	}
	_session.set_dirty ();
}	

void
Panner::set_automation_state (AutoState state)
{
	for (vector<StreamPanner*>::iterator i = begin(); i != end(); ++i) {
		(*i)->set_automation_state (state);
	}
	_session.set_dirty ();
}	

AutoState
Panner::automation_state () const
{
	if (!empty()) {
		return front()->automation().automation_state ();
	} else {
		return Auto_Off;
	}
}

AutoStyle
Panner::automation_style () const
{
	if (!empty()) {
		return front()->automation().automation_style ();
	} else {
		return Auto_Absolute;
	}
}

void
Panner::transport_stopped (nframes_t frame)
{
	for (vector<StreamPanner*>::iterator i = begin(); i != end(); ++i) {
		(*i)->transport_stopped (frame);
	}
}	

void
Panner::snapshot (nframes_t now)
{
	for (vector<StreamPanner*>::iterator i = begin(); i != end(); ++i) {
		(*i)->snapshot (now);
	}
}	

void
Panner::clear_automation ()
{
	for (vector<StreamPanner*>::iterator i = begin(); i != end(); ++i) {
		(*i)->automation().clear ();
	}
	_session.set_dirty ();
}	

struct PanPlugins {
    string name;
    uint32_t nouts;
    StreamPanner* (*factory)(Panner&);
};

PanPlugins pan_plugins[] = {
	{ EqualPowerStereoPanner::name, 2, EqualPowerStereoPanner::factory },
	{ Multi2dPanner::name, 3, Multi2dPanner::factory },
	{ string (""), 0, 0 }
};

XMLNode&
Panner::get_state (void)
{
	return state (true);
}

XMLNode&
Panner::state (bool full)
{
	XMLNode* root = new XMLNode (X_("Panner"));
	char buf[32];

	root->add_property (X_("linked"), (_linked ? "yes" : "no"));
	root->add_property (X_("link_direction"), enum_2_string (_link_direction));
	root->add_property (X_("bypassed"), (bypassed() ? "yes" : "no"));

	/* add each output */

	for (vector<Panner::Output>::iterator o = outputs.begin(); o != outputs.end(); ++o) {
		XMLNode* onode = new XMLNode (X_("Output"));
		snprintf (buf, sizeof (buf), "%.12g", (*o).x);
		onode->add_property (X_("x"), buf);
		snprintf (buf, sizeof (buf), "%.12g", (*o).y);
		onode->add_property (X_("y"), buf);
		root->add_child_nocopy (*onode);
	}

	for (vector<StreamPanner*>::const_iterator i = begin(); i != end(); ++i) {
		root->add_child_nocopy ((*i)->state (full));
	}

	return *root;
}

int
Panner::set_state (const XMLNode& node)
{
	XMLNodeList nlist;
	XMLNodeConstIterator niter;
	const XMLProperty *prop;
	uint32_t i;
	StreamPanner* sp;
	LocaleGuard lg (X_("POSIX"));

	clear ();
	outputs.clear ();

	if ((prop = node.property (X_("linked"))) != 0) {
		set_linked (string_is_affirmative (prop->value()));
	}


	if ((prop = node.property (X_("bypassed"))) != 0) {
		set_bypassed (string_is_affirmative (prop->value()));
	}

	if ((prop = node.property (X_("link_direction"))) != 0) {
		LinkDirection ld; /* here to provide type information */
		set_link_direction (LinkDirection (string_2_enum (prop->value(), ld)));
	}

	nlist = node.children();

	for (niter = nlist.begin(); niter != nlist.end(); ++niter) {
		if ((*niter)->name() == X_("Output")) {
			
			float x, y;
			
			prop = (*niter)->property (X_("x"));
			sscanf (prop->value().c_str(), "%g", &x);
			
			prop = (*niter)->property (X_("y"));
			sscanf (prop->value().c_str(), "%g", &y);
			
			outputs.push_back (Output (x, y));
		}
	}

	for (niter = nlist.begin(); niter != nlist.end(); ++niter) {

		if ((*niter)->name() == X_("StreamPanner")) {
		
			if ((prop = (*niter)->property (X_("type")))) {
				
				for (i = 0; pan_plugins[i].factory; ++i) {
					if (prop->value() == pan_plugins[i].name) {
						
						
						/* note that we assume that all the stream panners
						   are of the same type. pretty good
						   assumption, but its still an assumption.
						*/
						
						sp = pan_plugins[i].factory (*this);
						
						if (sp->set_state (**niter) == 0) {
							push_back (sp);
						}
						
						break;
					}
				}
				
				
				if (!pan_plugins[i].factory) {
					error << string_compose (_("Unknown panner plugin \"%1\" found in pan state - ignored"),
							  prop->value())
					      << endmsg;
				}

			} else {
				error << _("panner plugin node has no type information!")
				      << endmsg;
				return -1;
			}

		} 	
	}

	/* don't try to do old-school automation loading if it wasn't marked as existing */

	if ((prop = node.property (X_("automation")))) {

		/* automation path is relative */
		
		automation_path = Glib::build_filename(_session.automation_dir(), prop->value ());
	} 

	return 0;
}



bool
Panner::touching () const
{
	for (vector<StreamPanner*>::const_iterator i = begin(); i != end(); ++i) {
		if ((*i)->automation().touching ()) {
			return true;
		}
	}

	return false;
}

void
Panner::set_position (float xpos, StreamPanner& orig)
{
	float xnow;
	float xdelta ;
	float xnew;

	orig.get_position (xnow);
	xdelta = xpos - xnow;
	
	if (_link_direction == SameDirection) {

		for (vector<StreamPanner*>::iterator i = begin(); i != end(); ++i) {
			if (*i == &orig) {
				(*i)->set_position (xpos, true);
			} else {
				(*i)->get_position (xnow);
				xnew = min (1.0f, xnow + xdelta);
				xnew = max (0.0f, xnew);
				(*i)->set_position (xnew, true);
			}
		}

	} else {

		for (vector<StreamPanner*>::iterator i = begin(); i != end(); ++i) {
			if (*i == &orig) {
				(*i)->set_position (xpos, true);
			} else {
				(*i)->get_position (xnow);
				xnew = min (1.0f, xnow - xdelta);
				xnew = max (0.0f, xnew);
				(*i)->set_position (xnew, true);
			}
		}
	}
}

void
Panner::set_position (float xpos, float ypos, StreamPanner& orig)
{
	float xnow, ynow;
	float xdelta, ydelta;
	float xnew, ynew;

	orig.get_position (xnow, ynow);
	xdelta = xpos - xnow;
	ydelta = ypos - ynow;
	
	if (_link_direction == SameDirection) {

		for (vector<StreamPanner*>::iterator i = begin(); i != end(); ++i) {
			if (*i == &orig) {
				(*i)->set_position (xpos, ypos, true);
			} else {
				(*i)->get_position (xnow, ynow);

				xnew = min (1.0f, xnow + xdelta);
				xnew = max (0.0f, xnew);

				ynew = min (1.0f, ynow + ydelta);
				ynew = max (0.0f, ynew);

				(*i)->set_position (xnew, ynew, true);
			}
		}

	} else {

		for (vector<StreamPanner*>::iterator i = begin(); i != end(); ++i) {
			if (*i == &orig) {
				(*i)->set_position (xpos, ypos, true);
			} else {
				(*i)->get_position (xnow, ynow);
				
				xnew = min (1.0f, xnow - xdelta);
				xnew = max (0.0f, xnew);

				ynew = min (1.0f, ynow - ydelta);
				ynew = max (0.0f, ynew);

				(*i)->set_position (xnew, ynew, true);
			}
		}
	}
}

void
Panner::set_position (float xpos, float ypos, float zpos, StreamPanner& orig)
{
	float xnow, ynow, znow;
	float xdelta, ydelta, zdelta;
	float xnew, ynew, znew;

	orig.get_position (xnow, ynow, znow);
	xdelta = xpos - xnow;
	ydelta = ypos - ynow;
	zdelta = zpos - znow;

	if (_link_direction == SameDirection) {

		for (vector<StreamPanner*>::iterator i = begin(); i != end(); ++i) {
			if (*i == &orig) {
				(*i)->set_position (xpos, ypos, zpos, true);
			} else {
				(*i)->get_position (xnow, ynow, znow);
				
				xnew = min (1.0f, xnow + xdelta);
				xnew = max (0.0f, xnew);

				ynew = min (1.0f, ynow + ydelta);
				ynew = max (0.0f, ynew);

				znew = min (1.0f, znow + zdelta);
				znew = max (0.0f, znew);

				(*i)->set_position (xnew, ynew, znew, true);
			}
		}

	} else {

		for (vector<StreamPanner*>::iterator i = begin(); i != end(); ++i) {
			if (*i == &orig) {
				(*i)->set_position (xpos, ypos, true);
			} else {
				(*i)->get_position (xnow, ynow, znow);

				xnew = min (1.0f, xnow - xdelta);
				xnew = max (0.0f, xnew);

				ynew = min (1.0f, ynow - ydelta);
				ynew = max (0.0f, ynew);

				znew = min (1.0f, znow + zdelta);
				znew = max (0.0f, znew);

				(*i)->set_position (xnew, ynew, znew, true);
			}
		}
	}
}

/* old school automation handling */

void
Panner::set_name (string str)
{
	automation_path = Glib::build_filename(_session.automation_dir(), 
		_session.snap_name() + "-pan-" + legalize_for_path (str) + ".automation");
}

int
Panner::load ()
{
	char line[128];
	uint32_t linecnt = 0;
	float version;
	iterator sp;
	LocaleGuard lg (X_("POSIX"));

	if (automation_path.length() == 0) {
		return 0;
	}
	
	if (access (automation_path.c_str(), F_OK)) {
		return 0;
	}

	ifstream in (automation_path.c_str());

	if (!in) {
		error << string_compose (_("cannot open pan automation file %1 (%2)"),
				  automation_path, strerror (errno))
		      << endmsg;
		return -1;
	}

	sp = begin();

	while (in.getline (line, sizeof(line), '\n')) {

		if (++linecnt == 1) {
			if (memcmp (line, X_("version"), 7) == 0) {
				if (sscanf (line, "version %f", &version) != 1) {
					error << string_compose(_("badly formed version number in pan automation event file \"%1\""), automation_path) << endmsg;
					return -1;
				}
			} else {
				error << string_compose(_("no version information in pan automation event file \"%1\" (first line = %2)"), 
						 automation_path, line) << endmsg;
				return -1;
			}

			continue;
		}

		if (strlen (line) == 0 || line[0] == '#') {
			continue;
		}

		if (strcmp (line, "begin") == 0) {
			
			if (sp == end()) {
				error << string_compose (_("too many panner states found in pan automation file %1"),
						  automation_path)
				      << endmsg;
				return -1;
			}

			if ((*sp)->load (in, automation_path, linecnt)) {
				return -1;
			}
			
			++sp;
		}
	}

	return 0;
}
