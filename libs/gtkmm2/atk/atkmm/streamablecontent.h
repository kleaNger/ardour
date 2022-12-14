// -*- c++ -*-
// Generated by gtkmmproc -- DO NOT MODIFY!
#ifndef _ATKMM_STREAMABLECONTENT_H
#define _ATKMM_STREAMABLECONTENT_H

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


#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern "C"
{
  typedef struct _AtkStreamableContentIface AtkStreamableContentIface;
  typedef struct _AtkStreamableContent      AtkStreamableContent;
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct _AtkStreamableContent AtkStreamableContent;
typedef struct _AtkStreamableContentClass AtkStreamableContentClass;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Atk
{ class StreamableContent_Class; } // namespace Atk
namespace Atk
{

class Object;

/** The ATK interface which provides access to streamable content.
 */

class StreamableContent : public Glib::Interface
{
  
#ifndef DOXYGEN_SHOULD_SKIP_THIS

public:
  typedef StreamableContent CppObjectType;
  typedef StreamableContent_Class CppClassType;
  typedef AtkStreamableContent BaseObjectType;
  typedef AtkStreamableContentIface BaseClassType;

private:
  friend class StreamableContent_Class;
  static CppClassType streamablecontent_class_;

  // noncopyable
  StreamableContent(const StreamableContent&);
  StreamableContent& operator=(const StreamableContent&);

protected:
  StreamableContent(); // you must derive from this class
  explicit StreamableContent(AtkStreamableContent* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
  virtual ~StreamableContent();

  static void add_interface(GType gtype_implementer);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;
  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GObject.
  AtkStreamableContent*       gobj()       { return reinterpret_cast<AtkStreamableContent*>(gobject_); }

  ///Provides access to the underlying C GObject.  
  const AtkStreamableContent* gobj() const { return reinterpret_cast<AtkStreamableContent*>(gobject_); }

private:

public:

  
  /** Gets the number of mime types supported by this object.
   * @return A <tt>int</tt> which is the number of mime types supported by the object.
   */
  int get_n_mime_types() const;
  
  /** Gets the character string of the specified mime type. The first mime
   * type is at position 0, the second at position 1, and so on.
   * @param i A <tt>int</tt> representing the position of the mime type starting from 0.
   * @return : a gchar* representing the specified mime type; the caller
   * should not free the character string.
   */
  Glib::ustring get_mime_type(int i) const;
  
  /** Gets the content in the specified mime type.
   * @param mime_type A gchar* representing the mime type.
   * @return A G::IOChannel which contains the content in the specified mime
   * type.
   */
  Glib::RefPtr<Glib::IOChannel> get_stream(const Glib::ustring& mime_type);

protected:
    virtual int get_n_mime_types_vfunc() const;
    virtual const gchar* get_mime_type_vfunc(int i) const;
    virtual GIOChannel* get_stream_vfunc(const Glib::ustring& mime_type);                                                   


public:

public:
  //C++ methods used to invoke GTK+ virtual functions:

protected:
  //GTK+ Virtual Functions (override these to change behaviour):

  //Default Signal Handlers::


};

} // namespace Atk


namespace Glib
{
  /** @relates Atk::StreamableContent
   * @param object The C instance
   * @param take_copy False if the result should take ownership of the C instance. True if it should take a new copy or ref.
   * @result A C++ instance that wraps this C instance.
   */
  Glib::RefPtr<Atk::StreamableContent> wrap(AtkStreamableContent* object, bool take_copy = false);

} // namespace Glib

#endif /* _ATKMM_STREAMABLECONTENT_H */

