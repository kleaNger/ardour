// -*- c++ -*-
// Generated by gtkmmproc -- DO NOT MODIFY!
#ifndef _GTKMM_OBJECT_H
#define _GTKMM_OBJECT_H

#include <glibmm.h>

/* $Id$ */

/* Copyright (C) 1998-2002 The gtkmm Development Team
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
#include <gtkmm/base.h>
#include <gtkmmconfig.h>


#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct _GtkObject GtkObject;
typedef struct _GtkObjectClass GtkObjectClass;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Gtk
{ class Object_Class; } // namespace Gtk
namespace Gtk
{

class Object;

template<class T>
T* manage(T* obj)
{
  obj->set_manage();
  return obj;
}


class Object : public Glib::Object
{
  public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef Object CppObjectType;
  typedef Object_Class CppClassType;
  typedef GtkObject BaseObjectType;
  typedef GtkObjectClass BaseClassType;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  virtual ~Object();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

private:
  friend class Object_Class;
  static CppClassType object_class_;

  // noncopyable
  Object(const Object&);
  Object& operator=(const Object&);

protected:
  explicit Object(const Glib::ConstructParams& construct_params);
  explicit Object(GtkObject* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;
  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GtkObject.
  GtkObject*       gobj()       { return reinterpret_cast<GtkObject*>(gobject_); }

  ///Provides access to the underlying C GtkObject.
  const GtkObject* gobj() const { return reinterpret_cast<GtkObject*>(gobject_); }


public:
  //C++ methods used to invoke GTK+ virtual functions:

protected:
  //GTK+ Virtual Functions (override these to change behaviour):

  //Default Signal Handlers::


private:

  
public:
  //void shutdown(); //We probably don't need this.
  //void finalize(); //We probably don't need this.

  //void set_user_data(gpointer data);
  //gpointer get_user_data();

  virtual void set_manage();

  /** Anonymous User Data Pointer.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<void*> property_user_data() ;

/** Anonymous User Data Pointer.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<void*> property_user_data() const;


  bool is_managed_() const;

protected:

  void destroy_();

  // If you need it, give me an example. murrayc. -- Me too. daniel.
  //_WRAP_SIGNAL(void destroy(), "destroy")
  

  void _init_unmanage(bool is_toplevel = false);
  virtual void destroy_notify_(); //override.
  void disconnect_cpp_wrapper();
  void _destroy_c_instance();
  static void callback_destroy_(GObject* gobject, void* data); //only connected for a short time.

  // set if flags used by derived classes.
  bool referenced_; // = not managed.
  bool gobject_disposed_;


};

} // namespace Gtk


namespace Glib
{
  /** @relates Gtk::Object
   * @param object The C instance
   * @param take_copy False if the result should take ownership of the C instance. True if it should take a new copy or ref.
   * @result A C++ instance that wraps this C instance.
   */
  Gtk::Object* wrap(GtkObject* object, bool take_copy = false);
}
#endif /* _GTKMM_OBJECT_H */

