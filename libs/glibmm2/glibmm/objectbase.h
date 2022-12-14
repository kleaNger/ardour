// -*- c++ -*-
#ifndef _GLIBMM_OBJECTBASE_H
#define _GLIBMM_OBJECTBASE_H

/* $Id$ */

/* Copyright 2002 The gtkmm Development Team
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

#include <glibmm/signalproxy.h>
#include <glibmm/propertyproxy.h>
#include <sigc++/trackable.h>
#include <typeinfo>
#include <glibmmconfig.h>
#include <glibmm/debug.h>

GLIBMM_USING_STD(type_info)

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern "C" { typedef struct _GObject GObject; }
#endif


namespace Glib
{

#ifndef DOXYGEN_SHOULD_SKIP_THIS
class GSigConnectionNode;
#endif

//This inherits virtually from sigc::trackable so that people can multiply inherit glibmm classes from other sigc::trackable-derived classes.
//See bugzilla.gnome.org bug # 116280
class ObjectBase : virtual public sigc::trackable
{
protected:
  // Glib::ObjectBase is used as virtual base class.  This means the ObjectBase
  // ctor runs before all others -- either implicitly or explicitly.  Each of
  // the available ctors initializes custom_type_name_ in a different way:
  //
  // 1) default:      custom_type_name_ = "gtkmm__anonymous_custom_type"
  // 2) const char*:  custom_type_name_ = custom_type_name
  // 3) type_info:    custom_type_name_ = custom_type_info.name()
  //
  // All classes generated by gtkmmproc use ctor 2) with custom_type_name = 0,
  // which essentially means it's not a custom type.  This is used to optimize
  // vfunc and signal handler callbacks -- since the C++ virtual methods are
  // not overridden, invocation can be skipped.
  //
  // The default ctor 1) is called implicitly from the ctor of user-derived
  // classes -- yes, even if e.g. Gtk::Button calls ctor 2), a derived ctor
  // always overrides this choice.  The language itself ensures that the ctor
  // is only invoked once.
  //
  // Ctor 3) is a special feature to allow creation of derived types on the
  // fly, without having to use g_object_new() manually.  This feature is
  // sometimes necessary, e.g. to implement a custom Gtk::CellRenderer.  The
  // neat trick with the virtual base class ctor makes it possible to reuse
  // the same direct base class' ctor as with non-custom types.

  ObjectBase();
  explicit ObjectBase(const char* custom_type_name);
  explicit ObjectBase(const std::type_info& custom_type_info);

  virtual ~ObjectBase() = 0;

  // Called by Glib::Object and Glib::Interface constructors. See comments there.
  void initialize(GObject* castitem);

public:

  /// You probably want to use a specific property_*() accessor method instead.
  void set_property_value(const Glib::ustring& property_name, const Glib::ValueBase& value);

  /// You probably want to use a specific property_*() accessor method instead.
  void get_property_value(const Glib::ustring& property_name, Glib::ValueBase& value) const;

  /// You probably want to use a specific property_*() accessor method instead.
  template <class PropertyType>
  void set_property(const Glib::ustring& property_name, const PropertyType& value);

  /// You probably want to use a specific property_*() accessor method instead.
  template <class PropertyType>
  void get_property(const Glib::ustring& property_name, PropertyType& value) const;

  
  virtual void reference()   const;
  virtual void unreference() const;

  inline GObject*       gobj()       { return gobject_; }
  inline const GObject* gobj() const { return gobject_; }

  // Give a ref-ed copy to someone. Use for direct struct access.
  GObject* gobj_copy() const;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static ObjectBase* _get_current_wrapper(GObject* object);
  bool _cpp_destruction_is_in_progress() const;
#endif

protected:
  GObject*            gobject_; // the GLib/GDK/GTK+ object instance
  const char*         custom_type_name_;
  bool                cpp_destruction_in_progress_;

  bool is_anonymous_custom_() const;
  bool is_derived_() const;

  static  void destroy_notify_callback_(void* data);
  virtual void destroy_notify_();

  void _set_current_wrapper(GObject* object);

private:
  // noncopyable
  ObjectBase(const ObjectBase&);
  ObjectBase& operator=(const ObjectBase&);

  virtual void set_manage(); // calls g_error()

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  friend class Glib::GSigConnectionNode; // for GSigConnectionNode::notify()
#endif
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS

template <class PropertyType>
void ObjectBase::set_property(const Glib::ustring& property_name, const PropertyType& value)
{
  Glib::Value<PropertyType> property_value;
  property_value.init(Glib::Value<PropertyType>::value_type());

  property_value.set(value);
  this->set_property_value(property_name, property_value);
}

template <class PropertyType>
void ObjectBase::get_property(const Glib::ustring& property_name, PropertyType& value) const
{
  Glib::Value<PropertyType> property_value;
  property_value.init(Glib::Value<PropertyType>::value_type());

  this->get_property_value(property_name, property_value);

  value = property_value.get();
}

#endif /* DOXYGEN_SHOULD_SKIP_THIS */


bool _gobject_cppinstance_already_deleted(GObject* gobject);

} // namespace Glib


#endif /* _GLIBMM_OBJECTBASE_H */

