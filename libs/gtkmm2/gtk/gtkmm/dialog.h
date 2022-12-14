// -*- c++ -*-
// Generated by gtkmmproc -- DO NOT MODIFY!
#ifndef _GTKMM_DIALOG_H
#define _GTKMM_DIALOG_H

#include <glibmm.h>

/* $Id$ */

/* dialog.h
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

#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>


#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct _GtkDialog GtkDialog;
typedef struct _GtkDialogClass GtkDialogClass;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Gtk
{ class Dialog_Class; } // namespace Gtk
namespace Gtk
{

/** @defgroup Dialogs Dialogs
 */


/** @addtogroup gtkmmEnums Enums and Flags */

/**
 * @ingroup gtkmmEnums
 */
enum ResponseType
{
  RESPONSE_NONE = -1,
  RESPONSE_REJECT = -2,
  RESPONSE_ACCEPT = -3,
  RESPONSE_DELETE_EVENT = -4,
  RESPONSE_OK = -5,
  RESPONSE_CANCEL = -6,
  RESPONSE_CLOSE = -7,
  RESPONSE_YES = -8,
  RESPONSE_NO = -9,
  RESPONSE_APPLY = -10,
  RESPONSE_HELP = -11
};

} // namespace Gtk


#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Glib
{

template <>
class Value<Gtk::ResponseType> : public Glib::Value_Enum<Gtk::ResponseType>
{
public:
  static GType value_type() G_GNUC_CONST;
};

} // namespace Glib
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Gtk
{


/** Create popup windows.
 *
 * Dialog boxes are a convenient way to prompt the user for a small amount
 * of input, eg. to display a message, ask a question, or anything else that
 * does not require extensive effort on the user's part. 
 *
 * gtkmm treats a dialog as a window split vertically. The top section is a
 * Gtk::VBox, and is where widgets such as a Gtk::Label or a Gtk::Entry should be
 * packed. The bottom area is known as the action_area. This is generally
 * used for packing buttons into the dialog which may perform functions such
 * as cancel, ok, or apply. The two areas are separated by a Gtk::HSeparator. 
 *
 * The dialog can be 'modal' (that is, one which freezes the rest of the
 * application from user input) - this can be specified in the Gtk::Dialog
 * constructor.
 *
 * When adding buttons using add_button(), clicking the button will emit
 * signal_response() with a "response id" you specified. You are encouraged
 * to use the Gtk::ResponseType enum. If a dialog receives a delete event,
 * the "response" signal will be emitted with a response id of
 * Gtk::RESPONSE_NONE.
 *
 * If you want to block waiting for a dialog to return before returning control
 * flow to your code, you can call run(). This function enters a
 * recursive main loop and waits for the user to respond to the dialog, returning
 * the response ID corresponding to the button the user clicked. 
 *
 * @ingroup Dialogs
 */

class Dialog : public Window
{
  public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef Dialog CppObjectType;
  typedef Dialog_Class CppClassType;
  typedef GtkDialog BaseObjectType;
  typedef GtkDialogClass BaseClassType;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  virtual ~Dialog();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

private:
  friend class Dialog_Class;
  static CppClassType dialog_class_;

  // noncopyable
  Dialog(const Dialog&);
  Dialog& operator=(const Dialog&);

protected:
  explicit Dialog(const Glib::ConstructParams& construct_params);
  explicit Dialog(GtkDialog* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;
  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GtkObject.
  GtkDialog*       gobj()       { return reinterpret_cast<GtkDialog*>(gobject_); }

  ///Provides access to the underlying C GtkObject.
  const GtkDialog* gobj() const { return reinterpret_cast<GtkDialog*>(gobject_); }


public:
  //C++ methods used to invoke GTK+ virtual functions:

protected:
  //GTK+ Virtual Functions (override these to change behaviour):

  //Default Signal Handlers::
  virtual void on_response(int response_id);


private:

  
public:
  Dialog();
  explicit Dialog(const Glib::ustring& title, bool modal = false, bool use_separator = false);
  Dialog(const Glib::ustring& title, Gtk::Window& parent, bool modal = false, bool use_separator = false);

  
  /** Adds an activatable widget to the action area of a Gtk::Dialog,
   * connecting a signal handler that will emit the "response" signal on
   * the dialog when the widget is activated.  The widget is appended to
   * the end of the dialog's action area.  If you want to add a
   * non-activatable widget, simply pack it into the
   * <tt>action_area</tt> field of the Gtk::Dialog struct.
   * @param child An activatable widget.
   * @param response_id Response ID for @a child .
   */
  void add_action_widget(Widget& child, int response_id);
  
  /** Adds a button with the given text (or a stock button, if @a button_text  is a
   * stock ID) and sets things up so that clicking the button will emit the
   * "response" signal with the given @a response_id . The button is appended to the
   * end of the dialog's action area. The button widget is returned, but usually
   * you don't need it.
   * @param button_text Text of button, or stock ID.
   * @param response_id Response ID for the button.
   * @return The button widget that was added.
   */
  Button* add_button(const Glib::ustring& button_text, int response_id);
  
  /** Adds a button with the given text (or a stock button, if @a button_text  is a
   * stock ID) and sets things up so that clicking the button will emit the
   * "response" signal with the given @a response_id . The button is appended to the
   * end of the dialog's action area. The button widget is returned, but usually
   * you don't need it.
   * @param button_text Text of button, or stock ID.
   * @param response_id Response ID for the button.
   * @return The button widget that was added.
   */
  Button* add_button(const Gtk::StockID& stock_id, int response_id);
  
  /** Calls <tt>gtk_widget_set_sensitive (widget, @a setting )</tt> 
   * for each widget in the dialog's action area with the given @a response_id .
   * A convenient way to sensitize/desensitize dialog buttons.
   * @param response_id A response ID.
   * @param setting <tt>true</tt> for sensitive.
   */
  void set_response_sensitive(int response_id, bool setting = true);
  
  /** Sets the last widget in the dialog's action area with the given @a response_id 
   * as the default widget for the dialog. Pressing "Enter" normally activates
   * the default widget.
   * @param response_id A response ID.
   */
  void set_default_response(int response_id);
  
  /** Sets whether the dialog has a separator above the buttons.
   * <tt>true</tt> by default.
   * @param setting <tt>true</tt> to have a separator.
   */
  void set_has_separator(bool setting = true);
  
  /** Accessor for whether the dialog has a separator.
   * @return <tt>true</tt> if the dialog has a separator.
   */
  bool get_has_separator() const;
  
  
  /** Returns <tt>true</tt> if dialogs are expected to use an alternative
   * button order on the screen @a screen . See 
   * Gtk::Dialog::set_alternative_button_order() for more details
   * about alternative button order. 
   * 
   * If you need to use this function, you should probably connect
   * to the ::notify:gtk-alternative-button-order signal on the
   * Gtk::Settings object associated to @a screen , in order to be 
   * notified if the button order setting changes.
   * @param screen A Gdk::Screen, or <tt>0</tt> to use the default screen.
   * @return Whether the alternative button order should be used
   * 
   * Since: 2.6.
   */
  static bool alternative_button_order(const Glib::RefPtr<const Gdk::Screen>& screen);
  	
  //TODO: Document this:   
  void set_alternative_button_order_from_array(const Glib::ArrayHandle<int>& new_order);
  

  /** Emits the "response" signal with the given response ID. Used to
   * indicate that the user has responded to the dialog in some way;
   * typically either you or run() will be monitoring the
   * "response" signal and take appropriate action.
   * @param response_id Response ID.
   */
  void response(int response_id);
  
  /** Blocks in a recursive main loop until the @a dialog  emits the
   * response signal. It returns the response ID from the "response" signal emission.
   * Before entering the recursive main loop, run() calls
   * Gtk::Widget::show() on the dialog for you. Note that you still
   * need to show any children of the dialog yourself.
   * 
   * If the dialog receives "delete_event",  Gtk::Dialog::run() will return
   * Gtk::RESPONSE_DELETE_EVENT. Also, during Gtk::Dialog::run() the dialog will be
   * modal. You can force Gtk::Dialog::run() to return at any time by
   * calling Gtk::Dialog::response() to emit the "response"
   * signal.
   * 
   * After Gtk::Dialog::run() returns, you are responsible for hiding or
   * destroying the dialog if you wish to do so.
   * 
   * Typical usage of this function might be:
   * @code
   * <tt>int</tt> result = dialog.run();
   * switch (result)
   * {
   * case GTK_RESPONSE_ACCEPT:
   * do_application_specific_something (&lt;!-- --&gt;);
   * break;
   * default:
   * do_nothing_since_dialog_was_cancelled (&lt;!-- --&gt;);
   * break;
   * }
   * @endcode
   * @return Response ID.
   */
  int run();

  VBox* get_vbox();
  const VBox* get_vbox() const;
  HButtonBox* get_action_area();
  const HButtonBox* get_action_area() const;

  /** The dialog has a separator bar above its buttons.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<bool> property_has_separator() ;

/** The dialog has a separator bar above its buttons.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<bool> property_has_separator() const;


  Glib::SignalProxy1< void,int > signal_response();


protected:
  void construct_(bool modal, bool use_separator);


};

} /* namespace Gtk */


namespace Glib
{
  /** @relates Gtk::Dialog
   * @param object The C instance
   * @param take_copy False if the result should take ownership of the C instance. True if it should take a new copy or ref.
   * @result A C++ instance that wraps this C instance.
   */
  Gtk::Dialog* wrap(GtkDialog* object, bool take_copy = false);
}
#endif /* _GTKMM_DIALOG_H */

