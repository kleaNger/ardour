/*
    Copyright (C) 2003 Paul Davis 

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

#include <pango/pangoft2.h> // for fontmap resolution control for GnomeCanvas
#include <pango/pangocairo.h> // for fontmap resolution control for GnomeCanvas

#include <cstdlib>
#include <cctype>
#include <fstream>
#include <sys/stat.h>
#include <libart_lgpl/art_misc.h>
#include <gtkmm/rc.h>
#include <gtkmm/window.h>
#include <gtkmm/combo.h>
#include <gtkmm/label.h>
#include <gtkmm/paned.h>
#include <gtk/gtkpaned.h>

#include <gtkmm2ext/utils.h>
#include <ardour/ardour.h>
#include <ardour/configuration.h>

#include "ardour_ui.h"
#include "keyboard.h"
#include "utils.h"
#include "i18n.h"
#include "rgb_macros.h"
#include "canvas_impl.h"

using namespace std;
using namespace Gtk;
using namespace sigc;
using namespace Glib;
using namespace PBD;

sigc::signal<void>  DPIReset;

int
pixel_width (const string& str, Pango::FontDescription& font)
{
	if (str.empty()) {
		return 0;
	}

	Label foo;
	Glib::RefPtr<Pango::Layout> layout = foo.create_pango_layout ("");

	layout->set_font_description (font);
	layout->set_text (str);

	int width, height;
	Gtkmm2ext::get_ink_pixel_size (layout, width, height);
	return width;
}

string
fit_to_pixels (const string& str, int pixel_width, Pango::FontDescription& font, int& actual_width, bool with_ellipses)
{
	Label foo;
	Glib::RefPtr<Pango::Layout> layout = foo.create_pango_layout ("");
	string::size_type shorter_by = 0;
	string txt;

	layout->set_font_description (font);

	actual_width = 0;

	string ustr = str;
	string::iterator last = ustr.end();
	--last; /* now points at final entry */

	txt = ustr;

	while (!ustr.empty()) {

		layout->set_text (txt);

		int width, height;
		Gtkmm2ext::get_ink_pixel_size (layout, width, height);

		if (width < pixel_width) {
			actual_width = width;
			break;
		}
		
		ustr.erase (last--);
		shorter_by++;

		if (with_ellipses && shorter_by > 3) {
			txt = ustr;
			txt += "...";
		} else {
			txt = ustr;
		}
	}

	return txt;
}

gint
just_hide_it (GdkEventAny *ev, Gtk::Window *win)
{
	win->hide ();
	return TRUE;
}

/* xpm2rgb copied from nixieclock, which bore the legend:

    nixieclock - a nixie desktop timepiece
    Copyright (C) 2000 Greg Ercolano, erco@3dsite.com

    and was released under the GPL.
*/

unsigned char*
xpm2rgb (const char** xpm, uint32_t& w, uint32_t& h)
{
	static long vals[256], val;
	uint32_t t, x, y, colors, cpp;
	unsigned char c;
	unsigned char *savergb, *rgb;
	
	// PARSE HEADER
	
	if ( sscanf(xpm[0], "%u%u%u%u", &w, &h, &colors, &cpp) != 4 ) {
		error << string_compose (_("bad XPM header %1"), xpm[0])
		      << endmsg;
		return 0;
	}

	savergb = rgb = (unsigned char*) malloc (h * w * 3);
	
	// LOAD XPM COLORMAP LONG ENOUGH TO DO CONVERSION
	for (t = 0; t < colors; ++t) {
		sscanf (xpm[t+1], "%c c #%lx", &c, &val);
		vals[c] = val;
	}
	
	// COLORMAP -> RGB CONVERSION
	//    Get low 3 bytes from vals[]
	//

	const char *p;
	for (y = h-1; y > 0; --y) {

		for (p = xpm[1+colors+(h-y-1)], x = 0; x < w; x++, rgb += 3) {
			val = vals[(int)*p++];
			*(rgb+2) = val & 0xff; val >>= 8;  // 2:B
			*(rgb+1) = val & 0xff; val >>= 8;  // 1:G
			*(rgb+0) = val & 0xff;             // 0:R
		}
	}

	return (savergb);
}

unsigned char*
xpm2rgba (const char** xpm, uint32_t& w, uint32_t& h)
{
	static long vals[256], val;
	uint32_t t, x, y, colors, cpp;
	unsigned char c;
	unsigned char *savergb, *rgb;
	char transparent;

	// PARSE HEADER

	if ( sscanf(xpm[0], "%u%u%u%u", &w, &h, &colors, &cpp) != 4 ) {
		error << string_compose (_("bad XPM header %1"), xpm[0])
		      << endmsg;
		return 0;
	}

	savergb = rgb = (unsigned char*) malloc (h * w * 4);
	
	// LOAD XPM COLORMAP LONG ENOUGH TO DO CONVERSION

	if (strstr (xpm[1], "None")) {
		sscanf (xpm[1], "%c", &transparent);
		t = 1;
	} else {
		transparent = 0;
		t = 0;
	}

	for (; t < colors; ++t) {
		sscanf (xpm[t+1], "%c c #%lx", &c, &val);
		vals[c] = val;
	}
	
	// COLORMAP -> RGB CONVERSION
	//    Get low 3 bytes from vals[]
	//

	const char *p;
	for (y = h-1; y > 0; --y) {

		char alpha;

		for (p = xpm[1+colors+(h-y-1)], x = 0; x < w; x++, rgb += 4) {

			if (transparent && (*p++ == transparent)) {
				alpha = 0;
				val = 0;
			} else {
				alpha = 255;
				val = vals[(int)*p];
			}

			*(rgb+3) = alpha;                  // 3: alpha
			*(rgb+2) = val & 0xff; val >>= 8;  // 2:B
			*(rgb+1) = val & 0xff; val >>= 8;  // 1:G
			*(rgb+0) = val & 0xff;             // 0:R
		}
	}

	return (savergb);
}

ArdourCanvas::Points*
get_canvas_points (string who, uint32_t npoints)
{
	// cerr << who << ": wants " << npoints << " canvas points" << endl;
#ifdef TRAP_EXCESSIVE_POINT_REQUESTS
	if (npoints > (uint32_t) gdk_screen_width() + 4) {
		abort ();
	}
#endif
	return new ArdourCanvas::Points (npoints);
}

Pango::FontDescription*
get_font_for_style (string widgetname)
{
	Gtk::Window window (WINDOW_TOPLEVEL);
	Gtk::Label foobar;
	Glib::RefPtr<Gtk::Style> style;

	window.add (foobar);
	foobar.set_name (widgetname);
	foobar.ensure_style();

	style = foobar.get_style ();

	Glib::RefPtr<const Pango::Layout> layout = foobar.get_layout();
	
	PangoFontDescription *pfd = (PangoFontDescription *)pango_layout_get_font_description((PangoLayout *)layout->gobj());
	
	if (!pfd) {
		
		/* layout inherited its font description from a PangoContext */

		PangoContext* ctxt = (PangoContext*) pango_layout_get_context ((PangoLayout*) layout->gobj());
		pfd =  pango_context_get_font_description (ctxt);
		return new Pango::FontDescription (pfd, true); /* make a copy */
	} 

	return new Pango::FontDescription (pfd, true); /* make a copy */
}

uint32_t
rgba_from_style (string style, uint32_t r, uint32_t g, uint32_t b, uint32_t a, string attr, int state, bool rgba)
{
	/* In GTK+2, styles aren't set up correctly if the widget is not
	   attached to a toplevel window that has a screen pointer.
	*/

	static Gtk::Window* window = 0;

	if (window == 0) {
		window = new Window (WINDOW_TOPLEVEL);
	}

	Gtk::Label foo;
	
	window->add (foo);

	foo.set_name (style);
	foo.ensure_style ();
	
	GtkRcStyle* waverc = foo.get_style()->gobj()->rc_style;

	if (waverc) {
		if (attr == "fg") {
			r = waverc->fg[state].red / 257;
			g = waverc->fg[state].green / 257;
			b = waverc->fg[state].blue / 257;
 
			/* what a hack ... "a" is for "active" */
			if (state == Gtk::STATE_NORMAL && rgba) {
				a = waverc->fg[GTK_STATE_ACTIVE].red / 257;
			}
		} else if (attr == "bg") {
			r = g = b = 0;
			r = waverc->bg[state].red / 257;
			g = waverc->bg[state].green / 257;
			b = waverc->bg[state].blue / 257;
		} else if (attr == "base") {
			r = waverc->base[state].red / 257;
			g = waverc->base[state].green / 257;
			b = waverc->base[state].blue / 257;
		} else if (attr == "text") {
			r = waverc->text[state].red / 257;
			g = waverc->text[state].green / 257;
			b = waverc->text[state].blue / 257;
		}
	} else {
		warning << string_compose (_("missing RGBA style for \"%1\""), style) << endl;
	}

	window->remove ();
	
	if (state == Gtk::STATE_NORMAL && rgba) {
		return (uint32_t) RGBA_TO_UINT(r,g,b,a);
	} else {
		return (uint32_t) RGB_TO_UINT(r,g,b);
	}
}


Gdk::Color
color_from_style (string widget_style_name, int state, string attr)
{
	GtkStyle* style;

	style = gtk_rc_get_style_by_paths (gtk_settings_get_default(),
					   widget_style_name.c_str(),
					   0, G_TYPE_NONE);

	if (!style) {
		error << string_compose (_("no style found for %1, using red"), style) << endmsg;
		return Gdk::Color ("red");
	}

	cerr << "got style for " << widget_style_name << endl;

	if (attr == "fg") {
		return Gdk::Color (&style->fg[state]);
	}

	if (attr == "bg") {
		cerr << "returning color from bg\n";
		return Gdk::Color (&style->bg[state]);
	}

	if (attr == "light") {
		return Gdk::Color (&style->light[state]);
	}

	if (attr == "dark") {
		return Gdk::Color (&style->dark[state]);
	}

	if (attr == "mid") {
		return Gdk::Color (&style->mid[state]);
	}

	if (attr == "text") {
		return Gdk::Color (&style->text[state]);
	}

	if (attr == "base") {
		return Gdk::Color (&style->base[state]);
	}

	if (attr == "text_aa") {
		return Gdk::Color (&style->text_aa[state]);
	}

	error << string_compose (_("unknown style attribute %1 requested for color; using \"red\""), attr) << endmsg;
	return Gdk::Color ("red");
}


bool 
canvas_item_visible (ArdourCanvas::Item* item)
{
	return (item->gobj()->object.flags & GNOME_CANVAS_ITEM_VISIBLE) ? true : false;
}

void
set_color (Gdk::Color& c, int rgb)
{
	c.set_rgb((rgb >> 16)*256, ((rgb & 0xff00) >> 8)*256, (rgb & 0xff)*256);
}

bool
relay_key_press (GdkEventKey* ev, Gtk::Window* win)
{
	if (!key_press_focus_accelerator_handler (*win, ev)) {
		return PublicEditor::instance().on_key_press_event(ev);
	} else {
		return true;
	}
}

bool
key_press_focus_accelerator_handler (Gtk::Window& window, GdkEventKey* ev)
{
	GtkWindow* win = window.gobj();
	GtkWidget* focus = gtk_window_get_focus (win);
	bool special_handling_of_unmodified_accelerators = false;
	bool allow_activating = true;

#define DEBUG_ACCELERATOR_HANDLING
#ifdef  DEBUG_ACCELERATOR_HANDLING
	bool debug = (getenv ("ARDOUR_DEBUG_ACCELERATOR_HANDLING") != 0);
#endif
	if (focus) {
		if (GTK_IS_ENTRY(focus) || Keyboard::some_magic_widget_has_focus()) {
			special_handling_of_unmodified_accelerators = true;
		} 
#ifdef DEBUG_ACCELERATOR_HANDLING
		if (debug) {
			cerr << "Focus widget name " << gtk_widget_get_name(focus) << endl;
		}
#endif
	} 

#ifdef GTKOSX
	/* should this be universally true? */
	if (Keyboard::some_magic_widget_has_focus ()) {
		allow_activating = false;
	}
#endif

#ifdef DEBUG_ACCELERATOR_HANDLING
	if (debug) {
		cerr << "Win = " << win << " Key event: code = " << ev->keyval << " name = " << gdk_keyval_name (ev->keyval) << " state = " << hex << ev->state << dec 
		     << " ctrl " << ((ev->state & GDK_CONTROL_MASK) ? 1 : 0)
		     << " alt/mod1 " << ((ev->state & GDK_MOD1_MASK) ? 1 : 0)
		     << " mod2 " << ((ev->state & GDK_MOD2_MASK) ? 1 : 0)
		     << " shift " << ((ev->state & GDK_SHIFT_MASK) ? 1 : 0)
		     << " meta " << ((ev->state & GDK_META_MASK) ? 1 : 0)
		     << " lock " << ((ev->state & GDK_LOCK_MASK) ? 1 : 0)
		     << " special handling ? " 
		     << special_handling_of_unmodified_accelerators
		     << " magic widget focus ? "
		     << Keyboard::some_magic_widget_has_focus()
		     << " allow_activation ? "
		     << allow_activating
		     << endl;
	}
#endif

	/* This exists to allow us to override the way GTK handles
	   key events. The normal sequence is:

	   a) event is delivered to a GtkWindow
	   b) accelerators/mnemonics are activated
	   c) if (b) didn't handle the event, propagate to
	       the focus widget and/or focus chain

	   The problem with this is that if the accelerators include
	   keys without modifiers, such as the space bar or the 
	   letter "e", then pressing the key while typing into
	   a text entry widget results in the accelerator being
	   activated, instead of the desired letter appearing
	   in the text entry.

	   There is no good way of fixing this, but this
	   represents a compromise. The idea is that 
	   key events involving modifiers (not Shift)
	   get routed into the activation pathway first, then
	   get propagated to the focus widget if necessary.
	   
	   If the key event doesn't involve modifiers,
	   we deliver to the focus widget first, thus allowing
	   it to get "normal text" without interference
	   from acceleration.

	   Of course, this can also be problematic: if there
	   is a widget with focus, then it will swallow
	   all "normal text" accelerators.
	*/

	if (!special_handling_of_unmodified_accelerators) {

		/* pretend that certain key events that GTK does not allow
		   to be used as accelerators are actually something that
		   it does allow.
		*/

		uint32_t fakekey = ev->keyval;

		if (possibly_translate_keyval_to_make_legal_accelerator (fakekey)) {
#ifdef DEBUG_ACCELERATOR_HANDLING
			if (debug) {
				cerr << "\tactivate (was " << ev->keyval << " now " << fakekey << ") without special handling of unmodified accels\n";
			}
#endif
			GdkModifierType mod = GdkModifierType (ev->state);

			mod = GdkModifierType (mod & gtk_accelerator_get_default_mod_mask());
#ifdef GTKOSX
			/* GTK on OS X is currently (February 2012) setting both
			   the Meta and Mod2 bits in the event modifier state if 
			   the Command key is down.

			   gtk_accel_groups_activate() does not invoke any of the logic
			   that gtk_window_activate_key() will that sorts out that stupid
			   state of affairs, and as a result it fails to find a match
			   for the key event and the current set of accelerators.

			   to fix this, if the meta bit is set, remove the mod2 bit
			   from the modifier. this assumes that our bindings use Primary
			   which will have set the meta bit in the accelerator entry.
			*/
			if (mod & GDK_META_MASK) {
				mod = GdkModifierType (mod & ~GDK_MOD2_MASK);
			}
#endif
			if (allow_activating && gtk_accel_groups_activate(G_OBJECT(win), fakekey, mod)) {
#ifdef DEBUG_ACCELERATOR_HANDLING
				if (debug) {
					cerr << "\tactivation handled key\n";
				}
#endif
				return true;
			}
		}
	}

	/* consider all relevant modifiers but not LOCK or SHIFT */

	guint mask = (Keyboard::RelevantModifierKeyMask & ~(Gdk::SHIFT_MASK|Gdk::LOCK_MASK));

	if (!special_handling_of_unmodified_accelerators || (ev->state & mask)) {

		/* no special handling or there are modifiers in effect: accelerate first */

#ifdef DEBUG_ACCELERATOR_HANDLING
		if (debug) {
			cerr << "\tactivate, then propagate\n";
		}
#endif

		if (allow_activating) {
			if (gtk_window_activate_key (win, ev)) {
#ifdef DEBUG_ACCELERATOR_HANDLING
				if (debug) {
					cerr << "\tactivation handled key " << ev->keyval << endl;
				}
#endif
				
				return true;
			}
		}

#ifdef DEBUG_ACCELERATOR_HANDLING
		if (debug) {
			cerr << "\tnot accelerated, now propagate\n";
		}
#endif
		return gtk_window_propagate_key_event (win, ev);
	}
	
	/* no modifiers, propagate first */
	
#ifdef DEBUG_ACCELERATOR_HANDLING
	if (debug) {
		cerr << "\tpropagate, then activate\n";
	}
#endif
	if (!gtk_window_propagate_key_event (win, ev)) {
#ifdef DEBUG_ACCELERATOR_HANDLING
		if (debug) {
			cerr << "\tpropagation didn't handle, so activate\n";
		}
#endif

		if (allow_activating) {
			return gtk_window_activate_key (win, ev);
		} 
			
	} else {
#ifdef DEBUG_ACCELERATOR_HANDLING
		if (debug) {
			cerr << "\thandled by propagate\n";
		}
#endif
		return true;
	}

#ifdef DEBUG_ACCELERATOR_HANDLING
	if (debug) {
		cerr << "\tnot handled\n";
	}
#endif
	return true;
}

Glib::RefPtr<Gdk::Pixbuf>	
get_xpm (std::string name)
{
	//cerr << "xpm path = " << ARDOUR::find_data_file(name, "pixmaps") << endl;//DEBUG
	if (!xpm_map[name]) {
		try {
			xpm_map[name] = Gdk::Pixbuf::create_from_file (ARDOUR::find_data_file(name, "pixmaps"));
		}
		catch(const Glib::Error& e)	{
    		warning << "Caught Glib::Error: " << e.what() << endmsg;
  		}
	}
		
	return (xpm_map[name]);
}

Glib::RefPtr<Gdk::Pixbuf>	
get_icon (const char* cname)
{
	string name = cname;
	name += X_(".png");

	string path = ARDOUR::find_data_file (name, "icons");

	if (path.empty()) {
		fatal << string_compose (_("cannot find icon image for %1"), name) << endmsg;
		/*NOTREACHED*/
	}

	Glib::RefPtr<Gdk::Pixbuf> img;
	try {
		img = Gdk::Pixbuf::create_from_file (path);
	}
	catch (const Gdk::PixbufError &e)
    {
        cerr << "Caught PixbufError: " << e.what() << endl;
    }
    catch (...)
    {
        g_message("Caught ... ");
    }

	return img;
}

string
longest (vector<string>& strings)
{
	if (strings.empty()) {
		return string ("");
	}

	vector<string>::iterator longest = strings.begin();
	string::size_type longest_length = (*longest).length();
	
	vector<string>::iterator i = longest;
	++i;

	while (i != strings.end()) {
		
		string::size_type len = (*i).length();
		
		if (len > longest_length) {
			longest = i;
			longest_length = len;
		} 
		
		++i;
	}
	
	return *longest;
}

bool
key_is_legal_for_numeric_entry (guint keyval)
{
	switch (keyval) {
	case GDK_minus:
	case GDK_plus:
	case GDK_period:
	case GDK_comma:
	case GDK_0:
	case GDK_1:
	case GDK_2:
	case GDK_3:
	case GDK_4:
	case GDK_5:
	case GDK_6:
	case GDK_7:
	case GDK_8:
	case GDK_9:
	case GDK_KP_Add:
	case GDK_KP_Subtract:
	case GDK_KP_Decimal:
	case GDK_KP_0:
	case GDK_KP_1:
	case GDK_KP_2:
	case GDK_KP_3:
	case GDK_KP_4:
	case GDK_KP_5:
	case GDK_KP_6:
	case GDK_KP_7:
	case GDK_KP_8:
	case GDK_KP_9:
	case GDK_Return:
	case GDK_BackSpace:
	case GDK_Delete:
	case GDK_KP_Enter:
	case GDK_Home:
	case GDK_End:
	case GDK_Left:
	case GDK_Right:
		return true;
		
	default:
		break;
	}

	return false;
}
void
set_pango_fontsize ()
{
	long val = ARDOUR::Config->get_font_scale();

	/* FT2 rendering - used by GnomeCanvas, sigh */

	pango_ft2_font_map_set_resolution ((PangoFT2FontMap*) pango_ft2_font_map_for_display(), val/1024, val/1024);

	/* Cairo rendering, in case there is any */
	
	// pango_cairo_font_map_set_resolution ((PangoCairoFontMap*) pango_cairo_font_map_get_default(), val/1024);
}

void
reset_dpi ()
{
	long val = ARDOUR::Config->get_font_scale();
	set_pango_fontsize ();
	/* Xft rendering */

	gtk_settings_set_long_property (gtk_settings_get_default(),
					"gtk-xft-dpi", val, "ardour");
	DPIReset();//Emit Signal
}

bool
possibly_translate_keyval_to_make_legal_accelerator (uint32_t& keyval)
{
	int fakekey = GDK_VoidSymbol;

	switch (keyval) {
	case GDK_Tab:
	case GDK_ISO_Left_Tab:
		fakekey = GDK_nabla;
		break;
		
	case GDK_Up:
		fakekey = GDK_uparrow;
		break;
		
	case GDK_Down:
		fakekey = GDK_downarrow;
		break;
		
	case GDK_Right:
		fakekey = GDK_rightarrow;
		break;
		
	case GDK_Left:
		fakekey = GDK_leftarrow;
		break;
		
	default:
		break;
	}
	
	if (fakekey != GDK_VoidSymbol) {
		keyval = fakekey;
		return true;
	} 

	return false;
}
		
inline guint8
convert_color_channel (guint8 src,
		       guint8 alpha)
{
	return alpha ? ((guint (src) << 8) - src) / alpha : 0;
}


void
convert_bgra_to_rgba (guint8 const* src,
		      guint8*       dst,
		      int           width,
		      int           height)
{
	guint8 const* src_pixel = src;
	guint8*       dst_pixel = dst;
	
	for (int y = 0; y < height; y++) {
	  for (int x = 0; x < width; x++) {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	    /* Cairo [ B G R A ] is actually  [ B G R A ] in memory SOURCE
	                                        0 1 2 3
	       Pixbuf [ R G B A ] is actually [ R G B A ] in memory DEST
	    */
	    dst_pixel[0] = convert_color_channel (src_pixel[2],
						  src_pixel[3]); // R [0] <= [ 2 ]
	    dst_pixel[1] = convert_color_channel (src_pixel[1],
						  src_pixel[3]); // G [1] <= [ 1 ]
	    dst_pixel[2] = convert_color_channel (src_pixel[0],  
						  src_pixel[3]); // B [2] <= [ 0 ]
	    dst_pixel[3] = src_pixel[3]; // alpha
	    
#elif G_BYTE_ORDER == G_BIG_ENDIAN
	    /* Cairo [ B G R A ] is actually  [ A R G B ] in memory SOURCE
	                                        0 1 2 3
	       Pixbuf [ R G B A ] is actually [ R G B A ] in memory DEST
	    */
	    dst_pixel[0] = convert_color_channel (src_pixel[1],
						  src_pixel[0]); // R [0] <= [ 1 ]
	    dst_pixel[1] = convert_color_channel (src_pixel[2],
						  src_pixel[0]); // G [1] <= [ 2 ]
	    dst_pixel[2] = convert_color_channel (src_pixel[3],
						  src_pixel[0]); // B [2] <= [ 3 ]
	    dst_pixel[3] = src_pixel[0]; // alpha
	    
#else
#error ardour does not currently support PDP-endianess
#endif			

	    dst_pixel += 4;
	    src_pixel += 4;
	  }
	}
}

Glib::RefPtr<Gdk::Pixbuf>
pixbuf_from_string(const string& name, Pango::FontDescription* font, uint32_t rgba, int clip_width, int clip_height)
{
	Glib::RefPtr<Gdk::Pixbuf> buf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, clip_width, clip_height);
	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, clip_width, clip_height);
	cairo_t* cr = cairo_create (surface);
	cairo_text_extents_t te;

	float fr, fg, fb, fa;

	fr = ((rgba & 0xff000000) >> 24) / 255.0f;
	fg = ((rgba & 0xff0000) >> 16) / 255.0f;
	fb = ((rgba & 0xff00) >> 8) / 255.0f;
	fa = (rgba & 0xff) / 255.0f;

	cairo_set_source_rgba (cr, fr, fg, fb, fa);
	cairo_select_font_face (cr, font->get_family().c_str(),
				CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr,  font->get_size() / Pango::SCALE);
	cairo_text_extents (cr, name.c_str(), &te);
	
	cairo_move_to (cr, 0.5, 0.5 - te.height / 2 - te.y_bearing + clip_height / 2);
	cairo_show_text (cr, name.c_str());
	
	convert_bgra_to_rgba(cairo_image_surface_get_data (surface), buf->get_pixels(), clip_width, clip_height);

	cairo_destroy(cr);
	cairo_surface_destroy(surface);

	return buf;
}

int
physical_screen_height (Glib::RefPtr<Gdk::Window> win)
{
        GdkScreen* scr = gdk_screen_get_default();

        if (win) {
                GdkRectangle r;
                gint monitor = gdk_screen_get_monitor_at_window (scr, win->gobj());
                gdk_screen_get_monitor_geometry (scr, monitor, &r);
                return r.height;
        } else {
                return gdk_screen_get_height (scr);
        }
}

int
physical_screen_width (Glib::RefPtr<Gdk::Window> win)
{
        GdkScreen* scr = gdk_screen_get_default();
        
        if (win) {
                GdkRectangle r;
                gint monitor = gdk_screen_get_monitor_at_window (scr, win->gobj());
                gdk_screen_get_monitor_geometry (scr, monitor, &r);
                return r.width;
        } else {
                return gdk_screen_get_width (scr);
        }
}
