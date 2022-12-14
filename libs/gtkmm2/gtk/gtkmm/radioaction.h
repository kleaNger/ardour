// -*- c++ -*-
// Generated by gtkmmproc -- DO NOT MODIFY!
#ifndef _GTKMM_RADIOACTION_H
#define _GTKMM_RADIOACTION_H

#include <glibmm.h>

/* $Id$ */

/* Copyright (C) 2003 The gtkmm Development Team
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

#include <gtkmm/toggleaction.h>
#include <gtkmm/radiobuttongroup.h>


#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct _GtkRadioAction GtkRadioAction;
typedef struct _GtkRadioActionClass GtkRadioActionClass;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Gtk
{ class RadioAction_Class; } // namespace Gtk
namespace Gtk
{


class RadioAction : public Gtk::ToggleAction
{
  
#ifndef DOXYGEN_SHOULD_SKIP_THIS

public:
  typedef RadioAction CppObjectType;
  typedef RadioAction_Class CppClassType;
  typedef GtkRadioAction BaseObjectType;
  typedef GtkRadioActionClass BaseClassType;

private:  friend class RadioAction_Class;
  static CppClassType radioaction_class_;

private:
  // noncopyable
  RadioAction(const RadioAction&);
  RadioAction& operator=(const RadioAction&);

protected:
  explicit RadioAction(const Glib::ConstructParams& construct_params);
  explicit RadioAction(GtkRadioAction* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
  virtual ~RadioAction();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;
  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GObject.
  GtkRadioAction*       gobj()       { return reinterpret_cast<GtkRadioAction*>(gobject_); }

  ///Provides access to the underlying C GObject.
  const GtkRadioAction* gobj() const { return reinterpret_cast<GtkRadioAction*>(gobject_); }

  ///Provides access to the underlying C instance. The caller is responsible for unrefing it. Use when directly setting fields in structs.
  GtkRadioAction* gobj_copy();

private:

public:
  typedef RadioButtonGroup Group;
  
protected:
  RadioAction();
  explicit RadioAction(Group& group, const Glib::ustring& name, const StockID& stock_id, const Glib::ustring& label = Glib::ustring(), const Glib::ustring& tooltip = Glib::ustring());

public:
  
  static Glib::RefPtr<RadioAction> create();

  static Glib::RefPtr<RadioAction> create(Group& group, const Glib::ustring& name, const Glib::ustring& label =  Glib::ustring(), const Glib::ustring& tooltip = Glib::ustring());
  static Glib::RefPtr<RadioAction> create(Group& group, const Glib::ustring& name, const Gtk::StockID& stock_id, const Glib::ustring& label = Glib::ustring(), const Glib::ustring& tooltip =  Glib::ustring());

  
  /** Returns the list representing the radio group for this object
   * @return The list representing the radio group for this object
   * 
   * Since: 2.4.
   */
  Group get_group();
   void set_group(Group& group);
  
  
  /** Obtains the value property of the the currently active member of 
   * the group to which @a action  belongs.
   * @return The value of the currently active group member
   * 
   * Since: 2.4.
   */
  int get_current_value() const;

  
  Glib::SignalProxy1< void,const Glib::RefPtr<RadioAction>& > signal_changed();


  /** The value returned by gtk_radio_action_get_current_value when this action is the current action of its group.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<int> property_value() ;

/** The value returned by gtk_radio_action_get_current_value when this action is the current action of its group.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<int> property_value() const;


public:

public:
  //C++ methods used to invoke GTK+ virtual functions:

protected:
  //GTK+ Virtual Functions (override these to change behaviour):

  //Default Signal Handlers::
  virtual void on_changed(const Glib::RefPtr<RadioAction>& current);


};

} // namespace Gtk


namespace Glib
{
  /** @relates Gtk::RadioAction
   * @param object The C instance
   * @param take_copy False if the result should take ownership of the C instance. True if it should take a new copy or ref.
   * @result A C++ instance that wraps this C instance.
   */
  Glib::RefPtr<Gtk::RadioAction> wrap(GtkRadioAction* object, bool take_copy = false);
}


#endif /* _GTKMM_RADIOACTION_H */

