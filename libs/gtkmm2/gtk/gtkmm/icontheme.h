// -*- c++ -*-
// Generated by gtkmmproc -- DO NOT MODIFY!
#ifndef _GTKMM_ICONTHEME_H
#define _GTKMM_ICONTHEME_H

#include <glibmm.h>

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

#include <gdkmm/pixbuf.h>
#include <gdkmm/screen.h>

#include <gtkmm/iconinfo.h>
 

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct _GtkIconTheme GtkIconTheme;
typedef struct _GtkIconThemeClass GtkIconThemeClass;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Gtk
{ class IconTheme_Class; } // namespace Gtk
namespace Gtk
{

/** @addtogroup gtkmmEnums Enums and Flags */

/**
 * @ingroup gtkmmEnums
 * @par Bitwise operators:
 * <tt>%IconLookupFlags operator|(IconLookupFlags, IconLookupFlags)</tt><br>
 * <tt>%IconLookupFlags operator&(IconLookupFlags, IconLookupFlags)</tt><br>
 * <tt>%IconLookupFlags operator^(IconLookupFlags, IconLookupFlags)</tt><br>
 * <tt>%IconLookupFlags operator~(IconLookupFlags)</tt><br>
 * <tt>%IconLookupFlags& operator|=(IconLookupFlags&, IconLookupFlags)</tt><br>
 * <tt>%IconLookupFlags& operator&=(IconLookupFlags&, IconLookupFlags)</tt><br>
 * <tt>%IconLookupFlags& operator^=(IconLookupFlags&, IconLookupFlags)</tt><br>
 */
enum IconLookupFlags
{
  ICON_LOOKUP_NO_SVG = 1 << 0,
  ICON_LOOKUP_FORCE_SVG = 1 << 1,
  ICON_LOOKUP_USE_BUILTIN = 1 << 2
};

/** @ingroup gtkmmEnums */
inline IconLookupFlags operator|(IconLookupFlags lhs, IconLookupFlags rhs)
  { return static_cast<IconLookupFlags>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs)); }

/** @ingroup gtkmmEnums */
inline IconLookupFlags operator&(IconLookupFlags lhs, IconLookupFlags rhs)
  { return static_cast<IconLookupFlags>(static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs)); }

/** @ingroup gtkmmEnums */
inline IconLookupFlags operator^(IconLookupFlags lhs, IconLookupFlags rhs)
  { return static_cast<IconLookupFlags>(static_cast<unsigned>(lhs) ^ static_cast<unsigned>(rhs)); }

/** @ingroup gtkmmEnums */
inline IconLookupFlags operator~(IconLookupFlags flags)
  { return static_cast<IconLookupFlags>(~static_cast<unsigned>(flags)); }

/** @ingroup gtkmmEnums */
inline IconLookupFlags& operator|=(IconLookupFlags& lhs, IconLookupFlags rhs)
  { return (lhs = static_cast<IconLookupFlags>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs))); }

/** @ingroup gtkmmEnums */
inline IconLookupFlags& operator&=(IconLookupFlags& lhs, IconLookupFlags rhs)
  { return (lhs = static_cast<IconLookupFlags>(static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs))); }

/** @ingroup gtkmmEnums */
inline IconLookupFlags& operator^=(IconLookupFlags& lhs, IconLookupFlags rhs)
  { return (lhs = static_cast<IconLookupFlags>(static_cast<unsigned>(lhs) ^ static_cast<unsigned>(rhs))); }


/** Exception class for Gdk::IconTheme errors.
 */
class IconThemeError : public Glib::Error
{
public:
  enum Code
  {
    ICON_THEME_NOT_FOUND,
    ICON_THEME_FAILED
  };

  IconThemeError(Code error_code, const Glib::ustring& error_message);
  explicit IconThemeError(GError* gobject);
  Code code() const;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
private:
  static void throw_func(GError* gobject);
  friend void wrap_init(); // uses throw_func()
#endif
};

} // namespace Gtk

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Glib
{

template <>
class Value<Gtk::IconThemeError::Code> : public Glib::Value_Enum<Gtk::IconThemeError::Code>
{
public:
  static GType value_type() G_GNUC_CONST;
};

} // namespace Glib
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Gtk
{

  
class IconTheme : public Glib::Object
{
  
#ifndef DOXYGEN_SHOULD_SKIP_THIS

public:
  typedef IconTheme CppObjectType;
  typedef IconTheme_Class CppClassType;
  typedef GtkIconTheme BaseObjectType;
  typedef GtkIconThemeClass BaseClassType;

private:  friend class IconTheme_Class;
  static CppClassType icontheme_class_;

private:
  // noncopyable
  IconTheme(const IconTheme&);
  IconTheme& operator=(const IconTheme&);

protected:
  explicit IconTheme(const Glib::ConstructParams& construct_params);
  explicit IconTheme(GtkIconTheme* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
  virtual ~IconTheme();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;
  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GObject.
  GtkIconTheme*       gobj()       { return reinterpret_cast<GtkIconTheme*>(gobject_); }

  ///Provides access to the underlying C GObject.
  const GtkIconTheme* gobj() const { return reinterpret_cast<GtkIconTheme*>(gobject_); }

  ///Provides access to the underlying C instance. The caller is responsible for unrefing it. Use when directly setting fields in structs.
  GtkIconTheme* gobj_copy();

private:

  
protected:
  IconTheme();
public:
  
  static Glib::RefPtr<IconTheme> create();

  
  /** Gets the icon theme for the default screen. See
   * get_for_screen().
   * @return A unique Gtk::IconTheme associated with
   * the default screen. This icon theme is associated with
   * the screen and can be used as long as the screen
   * is open.
   * 
   * Since: 2.4.
   */
  static Glib::RefPtr<IconTheme> get_default();
  
  /** Gets the icon theme object associated with @a screen ; if this
   * function has not previously been called for the given
   * screen, a new icon theme object will be created and
   * associated with the screen. Icon theme objects are
   * fairly expensive to create, so using this function
   * is usually a better choice than calling than new()
   * and setting the screen yourself; by using this function
   * a single icon theme object will be shared between users.
   * @param screen A Gdk::Screen.
   * @return A unique Gtk::IconTheme associated with
   * the given screen. This icon theme is associated with
   * the screen and can be used as long as the screen
   * is open.
   * 
   * Since: 2.4.
   */
  static Glib::RefPtr<IconTheme> get_for_screen(const Glib::RefPtr<Gdk::Screen>& screen);
  
  /** Sets the screen for an icon theme; the screen is used
   * to track the user's currently configured icon theme,
   * which might be different for different screens.
   * 
   * Since: 2.4
   * @param screen A Gdk::Screen.
   */
  void set_screen(const Glib::RefPtr<Gdk::Screen>& screen);
  void set_search_path(const Glib::ArrayHandle<Glib::ustring>& path);
  Glib::ArrayHandle<Glib::ustring> get_search_path() const;
  
  /** Appends a directory to the search path. 
   * See set_search_path(). 
   * 
   * Since: 2.4
   * @param path Directory name to append to the icon path.
   */
  void append_search_path(const Glib::ustring& path);
  
  /** Prepends a directory to the search path. 
   * See set_search_path().
   * 
   * Since: 2.4
   * @param path Directory name to prepend to the icon path.
   */
  void prepend_search_path(const Glib::ustring& path);
  
  /** Sets the name of the icon theme that the Gtk::IconTheme object uses
   * overriding system configuration. This function cannot be called
   * on the icon theme objects returned from get_default()
   * and get_default().
   * 
   * Since: 2.4
   * @param theme_name Name of icon theme to use instead of configured theme.
   */
  void set_custom_theme(const Glib::ustring& theme_name);
  
  /** Checks whether an icon theme includes an icon
   * for a particular name.
   * @param icon_name The name of an icon.
   * @return <tt>true</tt> if @a icon_theme  includes an
   * icon for @a icon_name .
   * 
   * Since: 2.4.
   */
  bool has_icon(const Glib::ustring& icon_name) const;

  Glib::ArrayHandle<int> get_icon_sizes(const Glib::ustring& icon_name) const;
  
  
//TODO: Update the docs for this, to suggest use of IconInfo::operator bool() instead of saying that it returns null.
  
  /** Looks up a named icon and returns a structure containing
   * information such as the filename of the icon. The icon
   * can then be rendered into a pixbuf using
   * gtk_icon_info_load_icon(). (load_icon()
   * combines these two steps if all you need is the pixbuf.)
   * @param icon_name The name of the icon to lookup.
   * @param size Desired icon size.
   * @param flags Flags modifying the behavior of the icon lookup.
   * @return A Gtk::IconInfo structure containing information
   * about the icon, or <tt>0</tt> if the icon wasn't found. Free with
   * gtk_icon_info_free()
   * 
   * Since: 2.4.
   */
  IconInfo lookup_icon(const Glib::ustring& icon_name, int size, IconLookupFlags flags) const;

  
  /** Looks up an icon in an icon theme, scales it to the given size
   * and renders it into a pixbuf. This is a convenience function;
   * if more details about the icon are needed, use
   * lookup_icon() followed by gtk_icon_info_load_icon().
   * @param icon_name The name of the icon to lookup.
   * @param size The desired icon size. The resulting icon may not be exactly this size; see gtk_icon_info_load_icon().
   * @param flags Flags modifying the behavior of the icon lookup.
   * @return The rendered icon; this may be a newly created icon
   * or a new reference to an internal icon, so you must not modify
   * the icon. Use Glib::object_unref() to release your reference to the
   * icon. <tt>0</tt> if the icon isn't found.
   * 
   * Since: 2.4.
   */
  Glib::RefPtr<Gdk::Pixbuf> load_icon(const Glib::ustring& icon_name, int size, IconLookupFlags flags) const;

  
  /** Lists the icons in the current icon theme. Only a subset
   * of the icons can be listed by providing a context string.
   * The set of values for the context string is system dependent,
   * but will typically include such values as 'apps' and
   * 'mimetypes'.
   * @param context A string identifying a particular type of icon,
   * or <tt>0</tt> to list all icons.
   * @return A G::List list holding the names of all the
   * icons in the theme. You must first free each element
   * in the list with Glib::free(), then free the list itself
   * with Glib::list_free().
   * 
   * Since: 2.4.
   */
  Glib::ListHandle<Glib::ustring> list_icons(const Glib::ustring& context) const;
  
  /** Gets the name of an icon that is representative of the
   * current theme (for instance, to use when presenting
   * a list of themes to the user.)
   * @return The name of an example icon or <tt>0</tt>.
   * Free with Glib::free().
   * 
   * Since: 2.4.
   */
  Glib::ustring get_example_icon_name() const;
  
  /** Checks to see if the icon theme has changed; if it has, any
   * currently cached information is discarded and will be reloaded
   * next time @a icon_theme  is accessed.
   * @return <tt>true</tt> if the icon theme has changed and needed
   * to be reloaded.
   * 
   * Since: 2.4.
   */
  bool rescan_if_needed();
  
  /** Registers a built-in icon for icon theme lookups. The idea
   * of built-in icons is to allow an application or library
   * that uses themed icons to function requiring files to
   * be present in the file system. For instance, the default
   * images for all of GTK+'s stock icons are registered
   * as built-icons.
   * 
   * In general, if you use add_builtin_icon()
   * you should also install the icon in the icon theme, so
   * that the icon is generally available.
   * 
   * This function will generally be used with pixbufs loaded
   * via gdk_pixbuf_new_from_inline().
   * 
   * Since: 2.4
   * @param icon_name The name of the icon to register.
   * @param size The size at which to register the icon (different
   * images can be registered for the same icon name
   * at different sizes.).
   * @param pixbuf Gdk::Pixbuf that contains the image to use
   * for @a icon_name .
   */
  static void add_builtin_icon(const Glib::ustring& icon_name, int size, const Glib::RefPtr<Gdk::Pixbuf>& pixbuf);

  
  Glib::SignalProxy0< void > signal_changed();


public:

public:
  //C++ methods used to invoke GTK+ virtual functions:

protected:
  //GTK+ Virtual Functions (override these to change behaviour):

  //Default Signal Handlers::
  virtual void on_changed();


};

} // namespace Gtk


namespace Glib
{
  /** @relates Gtk::IconTheme
   * @param object The C instance
   * @param take_copy False if the result should take ownership of the C instance. True if it should take a new copy or ref.
   * @result A C++ instance that wraps this C instance.
   */
  Glib::RefPtr<Gtk::IconTheme> wrap(GtkIconTheme* object, bool take_copy = false);
}


#endif /* _GTKMM_ICONTHEME_H */

