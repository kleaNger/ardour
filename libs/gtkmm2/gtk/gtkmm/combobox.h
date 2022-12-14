// -*- c++ -*-
// Generated by gtkmmproc -- DO NOT MODIFY!
#ifndef _GTKMM_COMBOBOX_H
#define _GTKMM_COMBOBOX_H

#include <glibmm.h>

/* $Id$ */

/* combobox.h
 * 
 * Copyright (C) 2003 The gtkmm Development Team
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

#include <gtkmm/bin.h>
#include <gtkmm/celllayout.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/cellrenderer.h>
#include <gtkmm/treeview.h>


#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct _GtkComboBox GtkComboBox;
typedef struct _GtkComboBoxClass GtkComboBoxClass;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Gtk
{ class ComboBox_Class; } // namespace Gtk
namespace Gtk
{

/** A widget used to choose from a list of items.
 *
 * A ComboBox is a widget that allows the user to choose from a list of valid choices. The ComboBox displays the 
 * selected choice. When activated, the ComboBox displays a popup which allows the user to make a new choice. The 
 * style in which the selected value is displayed, and the style of the popup is determined by the current theme. 
 * It may be similar to a OptionMenu, or similar to a Windows-style combo box.
 *
 * The ComboBox uses the model-view pattern; the list of valid choices is specified in the form of a tree model, 
 * and the display of the choices can be adapted to the data in the model by using cell renderers, as you would in 
 * a tree view. This is possible since ComboBox implements the CellLayout interface. The tree model holding the 
 * valid choices is not restricted to a flat list, it can be a real tree, and the popup will reflect the tree 
 * structure.
 *
 * See also ComboBoxText, which is specialised for a single text column.
 *
 * @ingroup Widgets
 */

class ComboBox
: public Bin,
  public CellLayout
{
  public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef ComboBox CppObjectType;
  typedef ComboBox_Class CppClassType;
  typedef GtkComboBox BaseObjectType;
  typedef GtkComboBoxClass BaseClassType;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  virtual ~ComboBox();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

private:
  friend class ComboBox_Class;
  static CppClassType combobox_class_;

  // noncopyable
  ComboBox(const ComboBox&);
  ComboBox& operator=(const ComboBox&);

protected:
  explicit ComboBox(const Glib::ConstructParams& construct_params);
  explicit ComboBox(GtkComboBox* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;
  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GtkObject.
  GtkComboBox*       gobj()       { return reinterpret_cast<GtkComboBox*>(gobject_); }

  ///Provides access to the underlying C GtkObject.
  const GtkComboBox* gobj() const { return reinterpret_cast<GtkComboBox*>(gobject_); }


public:
  //C++ methods used to invoke GTK+ virtual functions:

protected:
  //GTK+ Virtual Functions (override these to change behaviour):

  //Default Signal Handlers::
  virtual void on_changed();


private:

  
public:
  ComboBox();
  
  /** Creates a new ComboBox with the model initialized to @a model.
   */
  explicit ComboBox(const Glib::RefPtr<TreeModel>& model);
   //See ComboBoxText for an equivalent of gtk_combo_box_new_text().


  /** Sets the wrap width of @a combo_box  to be @a width . The wrap width is basically
   * the preferred number of columns when you want the popup to be layed out
   * in a table.
   * 
   * Since: 2.4
   * @param width Preferred number of columns.
   */
  void set_wrap_width(int width);
  
  /** Returns the wrap width which is used to determine the number
   * of columns for the popup menu. If the wrap width is larger than
   * 1, the combo box is in table mode.
   * @return The wrap width.
   * 
   * Since: 2.6.
   */
  int get_wrap_width() const;

  
  /** Sets the column with row span information for @a combo_box  to be @a row_span .
   * The row span column contains integers which indicate how many rows
   * an item should span.
   * 
   * Since: 2.4
   * @param row_span A column in the model passed during construction.
   */
  void set_row_span_column(int row_span);
  
  /** Returns the column with row span information for @a combo_box .
   * @return The row span column.
   * 
   * Since: 2.6.
   */
  int get_row_span_column() const;

  
  /** Sets the column with column span information for @a combo_box  to be
   *  @a column_span . The column span column contains integers which indicate
   * how many columns an item should span.
   * 
   * Since: 2.4
   * @param column_span A column in the model passed during construction.
   */
  void set_column_span_column(int column_span);
  
  /** Returns the column with column span information for @a combo_box .
   * @return The column span column.
   * 
   * Since: 2.6.
   */
  int get_column_span_column() const;

  
  /** Gets the current value of the :add-tearoffs property.
   * @return The current value of the :add-tearoffs property.
   */
  bool get_add_tearoffs() const;
  
  /** Sets whether the popup menu should have a tearoff 
   * menu item.
   * 
   * Since: 2.6
   * @param add_tearoffs <tt>true</tt> to add tearoff menu items.
   */
  void set_add_tearoffs(bool add_tearoffs = true);

  
  /** Returns whether the combo box grabs focus when it is clicked 
   * with the mouse. See set_focus_on_click().
   * @return <tt>true</tt> if the combo box grabs focus when it is 
   * clicked with the mouse.
   * 
   * Since: 2.6.
   */
  bool get_focus_on_click() const;
  
  /** Sets whether the combo box will grab focus when it is clicked with 
   * the mouse. Making mouse clicks not grab focus is useful in places 
   * like toolbars where you don't want the keyboard focus removed from 
   * the main area of the application.
   * 
   * Since: 2.6
   * @param focus_on_click Whether the combo box grabs focus when clicked 
   * with the mouse.
   */
  void set_focus_on_click(bool focus_on_click = true);

/* get/set active item */
  
  /** Returns the index of the currently active item, or -1 if there's no
   * active item. If the model is a non-flat treemodel, and the active item 
   * is not an immediate child of the root of the tree, this function returns 
   * <tt>gtk_tree_path_get_indices (path)[0]</tt>, where 
   * <tt>path</tt> is the Gtk::TreePath of the active item.
   * @return An integer which is the index of the currently active item, or
   * -1 if there's no active item.
   * 
   * Since: 2.4.
   */
  int get_active_row_number() const;

  /** Gets an iterator that points to the current active item, if it exists.
   * @result The iterator.
   */
  TreeModel::iterator get_active();
  
  /** Gets an iterator that points to the current active item, if it exists.
   * @result The iterator.
   */
  TreeModel::const_iterator get_active() const;
  
    
  /** Sets the active item of @a combo_box  to be the item at @a index .
   * 
   * Since: 2.4
   * @param index An index in the model passed during construction, or -1 to have
   * no active item.
   */
  void set_active(int index);
  
  /** Sets the current active item to be the one referenced by @a iter . 
   *  @a iter  must correspond to a path of depth one.
   * 
   * Since: 2.4
   * @param iter The Gtk::TreeIter.
   */
  void set_active(const TreeModel::iterator& iter);
  
  /** Causes no item to be active. See also set_active().
   */
  void unset_active();

  
  /** Returns the Gtk::TreeModel which is acting as data source for @a combo_box .
   * @return A Gtk::TreeModel which was passed during construction.
   * 
   * Since: 2.4.
   */
  Glib::RefPtr<TreeModel> get_model();
  
  /** Returns the Gtk::TreeModel which is acting as data source for @a combo_box .
   * @return A Gtk::TreeModel which was passed during construction.
   * 
   * Since: 2.4.
   */
  Glib::RefPtr<const TreeModel> get_model() const;
  
  /** Sets the model used by @a combo_box  to be @a model . Will unset a previously set 
   * model (if applicable). If model is <tt>0</tt>, then it will unset the model.
   * 
   * Note that this function does not clear the cell renderers, you have to 
   * call gtk_combo_box_cell_layout_clear() yourself if you need to set up 
   * different cell renderers for the new model.
   * 
   * Since: 2.4
   * @param model A Gtk::TreeModel.
   */
  void set_model(const Glib::RefPtr<TreeModel>& model);

  typedef TreeView::SlotRowSeparator SlotRowSeparator;  
  
  /** Sets the row separator function, which is used to determine whether a row should be drawn as a separator. 
   * See also unset_row_separator_func().
   * 
   * @param slot The callback.
   */
  void set_row_separator_func(const SlotRowSeparator& slot);
  
  /** Causes no separators to be drawn.
   */
  void unset_row_separator_func();
  

  /** Pops up the menu or dropdown list of @a combo_box . 
   * 
   * This function is mostly intended for use by accessibility technologies;
   * applications should have little use for it.
   * 
   * Since: 2.4
   */
  void popup();
  
  /** Hides the menu or dropdown list of @a combo_box .
   * 
   * This function is mostly intended for use by accessibility technologies;
   * applications should have little use for it.
   * 
   * Since: 2.4
   */
  void popdown();
    
  
  /** Gets the accessible object corresponding to the combo box's popup.
   * 
   * This function is mostly intended for use by accessibility technologies;
   * applications should have little use for it.
   * @return The accessible object corresponding to the combo box's popup.
   * 
   * Since: 2.6.
   */
  Glib::RefPtr<Atk::Object> get_popup_accessible();
  
  /** Gets the accessible object corresponding to the combo box's popup.
   * 
   * This function is mostly intended for use by accessibility technologies;
   * applications should have little use for it.
   * @return The accessible object corresponding to the combo box's popup.
   * 
   * Since: 2.6.
   */
  Glib::RefPtr<const Atk::Object> get_popup_accessible() const;

   //These are in ComboBoxText.

  /** The model for the combo box.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy< Glib::RefPtr<TreeModel> > property_model() ;

/** The model for the combo box.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly< Glib::RefPtr<TreeModel> > property_model() const;

  /** Wrap width for layouting the items in a grid.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<int> property_wrap_width() ;

/** Wrap width for layouting the items in a grid.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<int> property_wrap_width() const;

  /** TreeModel column containing the row span values.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<int> property_row_span_column() ;

/** TreeModel column containing the row span values.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<int> property_row_span_column() const;

  /** TreeModel column containing the column span values.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<int> property_column_span_column() ;

/** TreeModel column containing the column span values.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<int> property_column_span_column() const;

  /** The item which is currently active.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<int> property_active() ;

/** The item which is currently active.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<int> property_active() const;

  /** Whether dropdowns should have a tearoff menu item.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<bool> property_add_tearoffs() ;

/** Whether dropdowns should have a tearoff menu item.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<bool> property_add_tearoffs() const;

  /** Whether the combo box draws a frame around the child.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<bool> property_has_frame() ;

/** Whether the combo box draws a frame around the child.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<bool> property_has_frame() const;

  /** Whether the combo box grabs focus when it is clicked with the mouse.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<bool> property_focus_on_click() ;

/** Whether the combo box grabs focus when it is clicked with the mouse.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<bool> property_focus_on_click() const;


  Glib::SignalProxy0< void > signal_changed();
                                             

};


} // namespace Gtk


namespace Glib
{
  /** @relates Gtk::ComboBox
   * @param object The C instance
   * @param take_copy False if the result should take ownership of the C instance. True if it should take a new copy or ref.
   * @result A C++ instance that wraps this C instance.
   */
  Gtk::ComboBox* wrap(GtkComboBox* object, bool take_copy = false);
}
#endif /* _GTKMM_COMBOBOX_H */

