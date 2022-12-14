// -*- c++ -*-
// Generated by gtkmmproc -- DO NOT MODIFY!
#ifndef _GTKMM_STOCKITEM_H
#define _GTKMM_STOCKITEM_H

#include <glibmm.h>

/* $Id$ */

/* Copyright(C) 1998-2002 The gtkmm Development Team
 *
 * This library is free software, ) you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, ) either
 * version 2 of the License, or(at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, ) without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library, ) if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <gdkmm/types.h>
#include <gtkmm/stockid.h>


#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern "C"
{
  typedef struct _GtkStockItem GtkStockItem;
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

namespace Gtk
{

class StockItem
{
  public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef StockItem CppObjectType;
  typedef GtkStockItem BaseObjectType;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  StockItem();

  // Use make_a_copy=true when getting it directly from a struct.
  explicit StockItem(GtkStockItem* castitem, bool make_a_copy = false);

  StockItem(const StockItem& src);
  StockItem& operator=(const StockItem& src);

  ~StockItem();

  GtkStockItem*       gobj()       { return gobject_; }
  const GtkStockItem* gobj() const { return gobject_; }

  ///Provides access to the underlying C instance. The caller is responsible for freeing it. Use when directly setting fields in structs.
  GtkStockItem* gobj_copy() const;

protected:
  GtkStockItem* gobject_;

private:


public:
  StockItem(const Gtk::StockID& stock_id, const Glib::ustring& label,
            Gdk::ModifierType modifier = Gdk::ModifierType(0), unsigned int keyval = 0,
            const Glib::ustring& translation_domain = Glib::ustring());

  StockID get_stock_id() const;
  Glib::ustring get_label() const;
  Gdk::ModifierType get_modifier() const;
  guint get_keyval() const;
  Glib::ustring get_translation_domain() const;

  static bool lookup(const Gtk::StockID& stock_id, Gtk::StockItem& item);


};

} // namespace Gtk


namespace Glib
{

  /** @relates Gtk::StockItem
   * @param object The C instance
   * @param take_copy False if the result should take ownership of the C instance. True if it should take a new copy or ref.
   * @result A C++ instance that wraps this C instance.
   */
Gtk::StockItem wrap(GtkStockItem* object, bool take_copy = false);

} // namespace Glib

#endif /* _GTKMM_STOCKITEM_H */

