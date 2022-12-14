// -*- c++ -*-
// Generated by gtkmmproc -- DO NOT MODIFY!
#ifndef _GDKMM_VISUAL_H
#define _GDKMM_VISUAL_H

#include <glibmm.h>

/* $Id$ */

/* bitmap.h
 *
 * Copyright (C) 1998-2002 The gtkmm Development Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <glibmm/object.h>
#include <gdkmm/screen.h>
#include <gdk/gdkvisual.h>


#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct _GdkVisual GdkVisual;
typedef struct _GdkVisualClass GdkVisualClass;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Gdk
{ class Visual_Class; } // namespace Gdk
namespace Gdk
{

class Screen;


/** @addtogroup gdkmmEnums Enums and Flags */

/**
 * @ingroup gdkmmEnums
 */
enum VisualType
{
  VISUAL_STATIC_GRAY,
  VISUAL_GRAYSCALE,
  VISUAL_STATIC_COLOR,
  VISUAL_PSEUDO_COLOR,
  VISUAL_TRUE_COLOR,
  VISUAL_DIRECT_COLOR
};

} // namespace Gdk


#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Glib
{

template <>
class Value<Gdk::VisualType> : public Glib::Value_Enum<Gdk::VisualType>
{
public:
  static GType value_type() G_GNUC_CONST;
};

} // namespace Glib
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Gdk
{


/** A Gdk::Visual describes a particular video hardware display format.
 * It includes information about the number of bits used for each color, the way the bits are translated into an RGB value
 * for display, and the way the bits are stored in memory. For example, a piece of display hardware might support 24-bit
 * color, 16-bit color, or 8-bit color; meaning 24/16/8-bit pixel sizes. For a given pixel size, pixels can be in different
 * formats; for example the "red" element of an RGB pixel may be in the top 8 bits of the pixel, or may be in the lower 4
 * bits.
 *
 * Usually you can avoid thinking about visuals in GTK+. Visuals are useful to interpret the contents of a GdkImage, but
 * you should avoid Gdk::Image precisely because its contents depend on the display hardware; use Gdk::Pixbuf instead, for
 * all but the most low-level purposes. Also, anytime you provide a Gdk::Colormap, the visual is implied as part of the
 * colormap (Gdk::Colormap::get_visual()), so you won't have to provide a visual in addition.
 *
 * There are several standard visuals. The visual returned by get_system() is the system's default visual. get_visual()
 * returns the visual most suited to displaying full-color image data. If you use the calls in Gdk::RGB, you should create
 * your windows using this visual (and the colormap returned by Gdk::Rgb::get_colormap()).
 *
 * A number of methods are provided for determining the "best" available visual. For the purposes of making this
 * determination, higher bit depths are considered better, and for visuals of the same bit depth, GDK_VISUAL_PSEUDO_COLOR
 * is preferred at 8bpp, otherwise, the visual types are ranked in the order of (highest to lowest) GDK_VISUAL_DIRECT_COLOR,
 * GDK_VISUAL_TRUE_COLOR, GDK_VISUAL_PSEUDO_COLOR, GDK_VISUAL_STATIC_COLOR, GDK_VISUAL_GRAYSCALE, then
 * GDK_VISUAL_STATIC_GRAY.
 */

class Visual : public Glib::Object
{
  
#ifndef DOXYGEN_SHOULD_SKIP_THIS

public:
  typedef Visual CppObjectType;
  typedef Visual_Class CppClassType;
  typedef GdkVisual BaseObjectType;
  typedef GdkVisualClass BaseClassType;

private:  friend class Visual_Class;
  static CppClassType visual_class_;

private:
  // noncopyable
  Visual(const Visual&);
  Visual& operator=(const Visual&);

protected:
  explicit Visual(const Glib::ConstructParams& construct_params);
  explicit Visual(GdkVisual* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
  virtual ~Visual();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;
  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GObject.
  GdkVisual*       gobj()       { return reinterpret_cast<GdkVisual*>(gobject_); }

  ///Provides access to the underlying C GObject.
  const GdkVisual* gobj() const { return reinterpret_cast<GdkVisual*>(gobject_); }

  ///Provides access to the underlying C instance. The caller is responsible for unrefing it. Use when directly setting fields in structs.
  GdkVisual* gobj_copy();

private:

protected:

  Visual();

public:
   
  /** Get the system'sdefault visual for the default GDK screen.
   * This is the visual for the root window of the display.
   * The return value should not be freed.
   * @return System visual.
   */
  static Glib::RefPtr<Visual> get_system();
   
  /** Get the visual with the most available colors for the default
   * GDK screen. The return value should not be freed.
   * @return Best visual.
   */
  static Glib::RefPtr<Visual> get_best();
   
  /** Get the best visual with depth @a depth  for the default GDK screen.
   * Color visuals and visuals with mutable colormaps are preferred
   * over grayscale or fixed-colormap visuals. The return value should not
   * be freed. <tt>0</tt> may be returned if no visual supports @a depth .
   * @param depth A bit depth.
   * @return Best visual for the given depth.
   */
  static Glib::RefPtr<Visual> get_best(int depth);
   
  /** Get the best visual of the given @a visual_type  for the default GDK screen.
   * Visuals with higher color depths are considered better. The return value
   * should not be freed. <tt>0</tt> may be returned if no visual has type
   *  @a visual_type .
   * @param visual_type A visual type.
   * @return Best visual of the given type.
   */
  static Glib::RefPtr<Visual> get_best(VisualType visual_type);
   
  /** Combines gdk_visual_get_best_with_depth() and gdk_visual_get_best_with_type().
   * @param depth A bit depth.
   * @param visual_type A visual type.
   * @return Best visual with both @a depth  and @a visual_type , or <tt>0</tt> if none.
   */
  static Glib::RefPtr<Visual> get_best(int depth, VisualType visual_type);

   
  /** Get the best available depth for the default GDK screen.  "Best"
   * means "largest," i.e. 32 preferred over 24 preferred over 8 bits
   * per pixel.
   * @return Best available depth.
   */
  static int get_best_depth();
   
  /** Return the best available visual type for the default GDK screen.
   * @return Best visual type.
   */
  static VisualType get_best_type();

   
  /** Gets the screen to which this visual belongs
   * @return The screen to which this visual belongs.
   * 
   * Since: 2.2.
   */
  Glib::RefPtr<Screen> get_screen();
   
  /** Gets the screen to which this visual belongs
   * @return The screen to which this visual belongs.
   * 
   * Since: 2.2.
   */
  Glib::RefPtr<const Screen> get_screen() const;


public:

public:
  //C++ methods used to invoke GTK+ virtual functions:

protected:
  //GTK+ Virtual Functions (override these to change behaviour):

  //Default Signal Handlers::


};

} //namespace Gdk


namespace Glib
{
  /** @relates Gdk::Visual
   * @param object The C instance
   * @param take_copy False if the result should take ownership of the C instance. True if it should take a new copy or ref.
   * @result A C++ instance that wraps this C instance.
   */
  Glib::RefPtr<Gdk::Visual> wrap(GdkVisual* object, bool take_copy = false);
}


#endif /* _GDKMM_VISUAL_H */

