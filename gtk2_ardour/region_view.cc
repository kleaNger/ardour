/*
    Copyright (C) 2001-2006 Paul Davis 

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
#include <cassert>
#include <algorithm>

#include <gtkmm.h>

#include <gtkmm2ext/gtk_ui.h>
#include <pbd/stacktrace.h>

#include <ardour/playlist.h>
#include <ardour/audioregion.h>
#include <ardour/audiosource.h>
#include <ardour/audio_diskstream.h>

#include "streamview.h"
#include "region_view.h"
#include "route_time_axis.h"
#include "simplerect.h"
#include "simpleline.h"
#include "waveview.h"
#include "public_editor.h"
#include "region_editor.h"
#include "ghostregion.h"
#include "route_time_axis.h"
#include "utils.h"
#include "rgb_macros.h"
#include "gui_thread.h"
#include "ardour_ui.h"

#include "i18n.h"

using namespace sigc;
using namespace ARDOUR;
using namespace PBD;
using namespace Editing;
using namespace ArdourCanvas;

static const int32_t sync_mark_width = 9;

sigc::signal<void,RegionView*> RegionView::RegionViewGoingAway;

RegionView::RegionView (ArdourCanvas::Group* parent, 
                        TimeAxisView&        tv,
                        boost::shared_ptr<ARDOUR::Region> r,
                        double               spu,
                        Gdk::Color&          basic_color
						)
	: TimeAxisViewItem (r->name(), *parent, tv, spu, basic_color, r->position(), r->length(), false,
			    TimeAxisViewItem::Visibility (TimeAxisViewItem::ShowNameText|
							  TimeAxisViewItem::ShowNameHighlight|
							  TimeAxisViewItem::ShowFrame))
	  , _region (r)
	  , sync_mark(0)
	  , sync_line(0)
	  , editor(0)
	  , current_visible_sync_position(0.0)
	  , valid(false)
	  , _pixel_width(1.0)
	  , in_destructor(false)
	  , wait_for_data(false)
{
}

RegionView::RegionView (const RegionView& other)
	: TimeAxisViewItem (other)
{
	/* derived concrete type will call init () */

	_region = other._region;
	current_visible_sync_position = other.current_visible_sync_position;
	valid = false;
	_pixel_width = other._pixel_width;
}

RegionView::RegionView (const RegionView& other, boost::shared_ptr<Region> other_region)
	: TimeAxisViewItem (other)
{
	/* this is a pseudo-copy constructor used when dragging regions 
	   around on the canvas.
	*/

	/* derived concrete type will call init () */

	_region = other_region;
	current_visible_sync_position = other.current_visible_sync_position;
	valid = false;
	_pixel_width = other._pixel_width;
}

RegionView::RegionView (ArdourCanvas::Group*         parent, 
                        TimeAxisView&                tv,
                        boost::shared_ptr<ARDOUR::Region> r,
                        double                       spu,
                        Gdk::Color&                  basic_color,
						bool recording,
                        TimeAxisViewItem::Visibility visibility)
	: TimeAxisViewItem (r->name(), *parent, tv, spu, basic_color, r->position(), r->length(), recording, visibility)
	, _region (r)
	, sync_mark(0)
	, sync_line(0)
	, editor(0)
	, current_visible_sync_position(0.0)
	, valid(false)
	, _pixel_width(1.0)
	, in_destructor(false)
	, wait_for_data(false)
{
}

void
RegionView::init (Gdk::Color& basic_color, bool wfd)
{
	editor        = 0;
	valid         = true;
	in_destructor = false;
	wait_for_data = wfd;
	sync_mark     = 0;
	sync_line     = 0;

	compute_colors (basic_color);

	if (name_highlight) {
		name_highlight->set_data ("regionview", this);
		name_highlight->signal_event().connect (bind (mem_fun (PublicEditor::instance(), &PublicEditor::canvas_region_view_name_highlight_event), name_highlight, this));
	}

	if (name_pixbuf) {
		name_pixbuf->set_data ("regionview", this);
	}

	reset_width_dependent_items ((double) _region->length() / samples_per_unit);

	set_height (trackview.current_height());

	_region->StateChanged.connect (mem_fun(*this, &RegionView::region_changed));

	group->signal_event().connect (bind (mem_fun (PublicEditor::instance(), &PublicEditor::canvas_region_view_event), group, this));

	set_colors ();

	ColorsChanged.connect (mem_fun (*this, &RegionView::color_handler));
	/* XXX sync mark drag? */
}

RegionView::~RegionView ()
{
	in_destructor = true;

	for (vector<GhostRegion*>::iterator g = ghosts.begin(); g != ghosts.end(); ++g) {
		delete *g;
	}

	if (editor) {
		delete editor;
	}
}

gint
RegionView::_lock_toggle (ArdourCanvas::Item* item, GdkEvent* ev, void* arg)
{
	switch (ev->type) {
	case GDK_BUTTON_RELEASE:
		static_cast<RegionView*>(arg)->lock_toggle ();
		return TRUE;
		break;
	default:
		break;
	} 
	return FALSE;
}

void
RegionView::lock_toggle ()
{
	_region->set_locked (!_region->locked());
}

void
RegionView::region_changed (Change what_changed)
{
	ENSURE_GUI_THREAD (bind (mem_fun(*this, &RegionView::region_changed), what_changed));

	if (what_changed & BoundsChanged) {
		region_resized (what_changed);
		region_sync_changed ();
	}
	if (what_changed & Region::MuteChanged) {
		region_muted ();
	}
	if (what_changed & Region::OpacityChanged) {
		region_opacity ();
	}
	if (what_changed & ARDOUR::NameChanged) {
		region_renamed ();
	}
	if (what_changed & Region::SyncOffsetChanged) {
		region_sync_changed ();
	}
	if (what_changed & Region::LayerChanged) {
		// this is handled by the playlist i believe
		//region_layered ();
	}
	if (what_changed & Region::LockChanged) {
		region_locked ();
	}
}

void
RegionView::region_locked ()
{
	/* name will show locked status */
	region_renamed ();
}

void
RegionView::region_resized (Change what_changed)
{
	double unit_length;

	if (what_changed & ARDOUR::PositionChanged) {
		set_position (_region->position(), 0);
	}

	if (what_changed & Change (StartChanged|LengthChanged)) {

		set_duration (_region->length(), 0);

		unit_length = _region->length() / samples_per_unit;
		
 		for (vector<GhostRegion*>::iterator i = ghosts.begin(); i != ghosts.end(); ++i) {

 			(*i)->set_duration (unit_length);

 		}
	}
}

void
RegionView::reset_width_dependent_items (double pixel_width)
{
	TimeAxisViewItem::reset_width_dependent_items (pixel_width);
	_pixel_width = pixel_width;
}

void
RegionView::region_layered ()
{
	RouteTimeAxisView *rtv = dynamic_cast<RouteTimeAxisView*>(&get_time_axis_view());
	assert(rtv);
	//rtv->view()->region_layered (this);
}
	
void
RegionView::region_muted ()
{
	set_frame_color ();
	region_renamed ();
}

void
RegionView::region_opacity ()
{
	set_frame_color ();
}

void
RegionView::raise ()
{
	_region->raise ();
}

void
RegionView::raise_to_top ()
{
	_region->raise_to_top ();
}

void
RegionView::lower ()
{
	_region->lower ();
}

void
RegionView::lower_to_bottom ()
{
	_region->lower_to_bottom ();
}

bool
RegionView::set_position (nframes_t pos, void* src, double* ignored)
{
	double delta;
	bool ret;

	if (!(ret = TimeAxisViewItem::set_position (pos, this, &delta))) {
		return false;
	}

	if (ignored) {
		*ignored = delta;
	}

	if (delta) {
		for (vector<GhostRegion*>::iterator i = ghosts.begin(); i != ghosts.end(); ++i) {
			(*i)->group->move (delta, 0.0);
		}
	}

	return ret;
}

void
RegionView::set_samples_per_unit (gdouble spu)
{
	TimeAxisViewItem::set_samples_per_unit (spu);

	for (vector<GhostRegion*>::iterator i = ghosts.begin(); i != ghosts.end(); ++i) {
		(*i)->set_samples_per_unit (spu);
		(*i)->set_duration (_region->length() / samples_per_unit);
	}

	region_sync_changed ();
}

bool
RegionView::set_duration (nframes_t frames, void *src)
{
	if (!TimeAxisViewItem::set_duration (frames, src)) {
		return false;
	}
	
	for (vector<GhostRegion*>::iterator i = ghosts.begin(); i != ghosts.end(); ++i) {
		(*i)->set_duration (_region->length() / samples_per_unit);
	}

	return true;
}

void
RegionView::compute_colors (Gdk::Color& basic_color)
{
	TimeAxisViewItem::compute_colors (basic_color);
}

void
RegionView::set_colors ()
{
	TimeAxisViewItem::set_colors ();
	
	if (sync_mark) {
		sync_mark->property_fill_color_rgba() = fill_color;
		sync_line->property_fill_color_rgba() = fill_color;
	}
}

void
RegionView::set_frame_color ()
{
	if (_region->opaque()) {
		fill_opacity = 130;
	} else {
		fill_opacity = 0;
	}

	TimeAxisViewItem::set_frame_color ();
}

void
RegionView::fake_set_opaque (bool yn)
{
       if (yn) {
               fill_opacity = 130;
       } else {
               fill_opacity = 0;
       }
       
       set_frame_color ();
}

void
RegionView::hide_region_editor()
{
	if (editor) {
		editor->hide_all ();
	}
}

void
RegionView::region_renamed ()
{
	string str;

	if (_region->locked()) {
		str += '>';
		str += _region->name();
		str += '<';
	} else {
		str = _region->name();
	}

	if (_region->speed_mismatch (trackview.session().frame_rate())) {
		str = string ("*") + str;
	}

	if (_region->muted()) {
		str = string ("!") + str;
	}

	set_item_name (str, this);
	set_name_text (str);
	reset_width_dependent_items (_pixel_width);
}

void
RegionView::region_sync_changed ()
{
	int sync_dir;
	nframes_t sync_offset;

	sync_offset = _region->sync_offset (sync_dir);

	if (sync_offset == 0) {
		/* no need for a sync mark */
		if (sync_mark) {
			sync_mark->hide();
			sync_line->hide ();
		}
		return;
	}

	if (!sync_mark) {

		/* points set below */
		
		sync_mark =  new ArdourCanvas::Polygon (*group);
		sync_mark->property_fill_color_rgba() = fill_color;

		sync_line = new ArdourCanvas::Line (*group);
		sync_line->property_fill_color_rgba() = fill_color;
		sync_line->property_width_pixels() = 1;
	}

	/* this has to handle both a genuine change of position, a change of samples_per_unit,
	   and a change in the bounds of the _region->
	 */

	if (sync_offset == 0) {

		/* no sync mark - its the start of the region */
		
		sync_mark->hide();
		sync_line->hide ();

	} else {

		if ((sync_dir < 0) || ((sync_dir > 0) && (sync_offset > _region->length()))) { 

			/* no sync mark - its out of the bounds of the region */

			sync_mark->hide();
			sync_line->hide ();

		} else {

			/* lets do it */

			Points points;
			
			//points = sync_mark->property_points().get_value();
			
			double offset = sync_offset / samples_per_unit;
			points.push_back (Gnome::Art::Point (offset - ((sync_mark_width-1)/2), 1));
			points.push_back (Gnome::Art::Point (offset + ((sync_mark_width-1)/2), 1));
			points.push_back (Gnome::Art::Point (offset, sync_mark_width - 1));
			points.push_back (Gnome::Art::Point (offset - ((sync_mark_width-1)/2), 1));	
			sync_mark->property_points().set_value (points);
			sync_mark->show ();

			points.clear ();
			points.push_back (Gnome::Art::Point (offset, 0));
			points.push_back (Gnome::Art::Point (offset, _height - NAME_HIGHLIGHT_SIZE));

			sync_line->property_points().set_value (points);
			sync_line->show ();
		}
	}
}

void
RegionView::move (double x_delta, double y_delta)
{
	if (_region->locked() || (x_delta == 0 && y_delta == 0)) {
		return;
	}

	get_canvas_group()->move (x_delta, y_delta);

	/* note: ghosts never leave their tracks so y_delta for them is always zero */

	for (vector<GhostRegion*>::iterator i = ghosts.begin(); i != ghosts.end(); ++i) {
		(*i)->group->move (x_delta, 0.0);
	}
}

void
RegionView::remove_ghost (GhostRegion* ghost)
{
	if (in_destructor) {
		return;
	}

	for (vector<GhostRegion*>::iterator i = ghosts.begin(); i != ghosts.end(); ++i) {
		if (*i == ghost) {
			ghosts.erase (i);
			break;
		}
	}
}

uint32_t
RegionView::get_fill_color ()
{
	return fill_color;
}

void
RegionView::set_height (double h)
{
	if (sync_line) {
		Points points;
		int sync_dir;
		nframes_t sync_offset;
		sync_offset = _region->sync_offset (sync_dir);
		double offset = sync_offset / samples_per_unit;

		points.push_back (Gnome::Art::Point (offset, 0));
		points.push_back (Gnome::Art::Point (offset, h - NAME_HIGHLIGHT_SIZE));
		sync_line->property_points().set_value (points);
	}
}
