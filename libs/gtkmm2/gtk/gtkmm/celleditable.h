// -*- c++ -*-
// Generated by gtkmmproc -- DO NOT MODIFY!
#ifndef _GTKMM_CELLEDITABLE_H
#define _GTKMM_CELLEDITABLE_H

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

#include <glibmm/interface.h>


#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern "C"
{
typedef struct _GtkCellEditableIface GtkCellEditableIface;
typedef union _GdkEvent GdkEvent;
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct _GtkCellEditable GtkCellEditable;
typedef struct _GtkCellEditableClass GtkCellEditableClass;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Gtk
{ class CellEditable_Class; } // namespace Gtk
namespace Gtk
{

/** Interface for widgets which are used for editing cells.
 * The CellEditable interface must be implemented for widgets to be usable when editing the contents of a TreeView cell. 
 */

class CellEditable : public Glib::Interface
{
  
#ifndef DOXYGEN_SHOULD_SKIP_THIS

public:
  typedef CellEditable CppObjectType;
  typedef CellEditable_Class CppClassType;
  typedef GtkCellEditable BaseObjectType;
  typedef GtkCellEditableIface BaseClassType;

private:
  friend class CellEditable_Class;
  static CppClassType celleditable_class_;

  // noncopyable
  CellEditable(const CellEditable&);
  CellEditable& operator=(const CellEditable&);

protected:
  CellEditable(); // you must derive from this class
  explicit CellEditable(GtkCellEditable* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
  virtual ~CellEditable();

  static void add_interface(GType gtype_implementer);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;
  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GObject.
  GtkCellEditable*       gobj()       { return reinterpret_cast<GtkCellEditable*>(gobject_); }

  ///Provides access to the underlying C GObject.  
  const GtkCellEditable* gobj() const { return reinterpret_cast<GtkCellEditable*>(gobject_); }

private:


public:
  
  /** Begins editing on a @a cell_editable .  @a event  is the Gdk::Event that began the
   * editing process.  It may be <tt>0</tt>, in the instance that editing was initiated
   * through programatic means.
   * @param event A Gdk::Event, or <tt>0</tt>.
   */
  void start_editing(GdkEvent* event);
  
  /** Emits the "editing_done" signal.  This signal is a sign for the cell renderer
   * to update its value from the cell.
   */
  void editing_done();
  
  /** Emits the "remove_widget" signal.  This signal is meant to indicate that the
   * cell is finished editing, and the widget may now be destroyed.
   */
  void remove_widget();

  
  Glib::SignalProxy0< void > signal_editing_done();

  
  Glib::SignalProxy0< void > signal_remove_widget();


protected:
    virtual void start_editing_vfunc(GdkEvent* event);


public:

public:
  //C++ methods used to invoke GTK+ virtual functions:

protected:
  //GTK+ Virtual Functions (override these to change behaviour):

  //Default Signal Handlers::
  virtual void on_editing_done();
  virtual void on_remove_widget();


};

} // namespace Gtk


namespace Glib
{
  /** @relates Gtk::CellEditable
   * @param object The C instance
   * @param take_copy False if the result should take ownership of the C instance. True if it should take a new copy or ref.
   * @result A C++ instance that wraps this C instance.
   */
  Glib::RefPtr<Gtk::CellEditable> wrap(GtkCellEditable* object, bool take_copy = false);

} // namespace Glib

#endif /* _GTKMM_CELLEDITABLE_H */

