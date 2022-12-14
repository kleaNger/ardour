// -*- c++ -*-
// Generated by gtkmmproc -- DO NOT MODIFY!
#ifndef _GTKMM_SIZEGROUP_H
#define _GTKMM_SIZEGROUP_H

#include <glibmm.h>

/* $Id$ */

/* box.h
 *
 * Copyright 2002 The gtkmm Development Team
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
#include <gtkmm/widget.h>


#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct _GtkSizeGroup GtkSizeGroup;
typedef struct _GtkSizeGroupClass GtkSizeGroupClass;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Gtk
{ class SizeGroup_Class; } // namespace Gtk
namespace Gtk
{


/** @addtogroup gtkmmEnums Enums and Flags */

/**
 * @ingroup gtkmmEnums
 */
enum SizeGroupMode
{
  SIZE_GROUP_NONE,
  SIZE_GROUP_HORIZONTAL,
  SIZE_GROUP_VERTICAL,
  SIZE_GROUP_BOTH
};

} // namespace Gtk


#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Glib
{

template <>
class Value<Gtk::SizeGroupMode> : public Glib::Value_Enum<Gtk::SizeGroupMode>
{
public:
  static GType value_type() G_GNUC_CONST;
};

} // namespace Glib
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Gtk
{


class SizeGroup : public Glib::Object
{
  
#ifndef DOXYGEN_SHOULD_SKIP_THIS

public:
  typedef SizeGroup CppObjectType;
  typedef SizeGroup_Class CppClassType;
  typedef GtkSizeGroup BaseObjectType;
  typedef GtkSizeGroupClass BaseClassType;

private:  friend class SizeGroup_Class;
  static CppClassType sizegroup_class_;

private:
  // noncopyable
  SizeGroup(const SizeGroup&);
  SizeGroup& operator=(const SizeGroup&);

protected:
  explicit SizeGroup(const Glib::ConstructParams& construct_params);
  explicit SizeGroup(GtkSizeGroup* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
  virtual ~SizeGroup();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;
  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GObject.
  GtkSizeGroup*       gobj()       { return reinterpret_cast<GtkSizeGroup*>(gobject_); }

  ///Provides access to the underlying C GObject.
  const GtkSizeGroup* gobj() const { return reinterpret_cast<GtkSizeGroup*>(gobject_); }

  ///Provides access to the underlying C instance. The caller is responsible for unrefing it. Use when directly setting fields in structs.
  GtkSizeGroup* gobj_copy();

private:

protected:
  explicit SizeGroup(SizeGroupMode mode);

public:

  
  static Glib::RefPtr<SizeGroup> create(SizeGroupMode mode);


  /** Sets the Gtk::SizeGroupMode of the size group. The mode of the size
   * group determines whether the widgets in the size group should
   * all have the same horizontal requisition (Gtk::SIZE_GROUP_MODE_HORIZONTAL)
   * all have the same vertical requisition (Gtk::SIZE_GROUP_MODE_VERTICAL),
   * or should all have the same requisition in both directions
   * (Gtk::SIZE_GROUP_MODE_BOTH).
   * @param mode The mode to set for the size group.
   */
  void set_mode(SizeGroupMode mode);
  
  /** Gets the current mode of the size group. See set_mode().
   * @return The current mode of the size group.
   */
  SizeGroupMode get_mode() const;
  
  /** Adds a widget to a Gtk::SizeGroup. In the future, the requisition
   * of the widget will be determined as the maximum of its requisition
   * and the requisition of the other widgets in the size group.
   * Whether this applies horizontally, vertically, or in both directions
   * depends on the mode of the size group. See set_mode().
   * @param widget The Gtk::Widget to add.
   */
  void add_widget(Widget& widget);
  
  /** Removes a widget from a Gtk::SizeGroup.
   * @param widget The Gtk::Widget to remove.
   */
  void remove_widget(Widget& widget);

  /** The directions in which the size group effects the requested sizes of its component widgets.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<SizeGroupMode> property_mode() ;

/** The directions in which the size group effects the requested sizes of its component widgets.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<SizeGroupMode> property_mode() const;


public:

public:
  //C++ methods used to invoke GTK+ virtual functions:

protected:
  //GTK+ Virtual Functions (override these to change behaviour):

  //Default Signal Handlers::


};

} /* namespace Gtk */


namespace Glib
{
  /** @relates Gtk::SizeGroup
   * @param object The C instance
   * @param take_copy False if the result should take ownership of the C instance. True if it should take a new copy or ref.
   * @result A C++ instance that wraps this C instance.
   */
  Glib::RefPtr<Gtk::SizeGroup> wrap(GtkSizeGroup* object, bool take_copy = false);
}


#endif /* _GTKMM_SIZEGROUP_H */

