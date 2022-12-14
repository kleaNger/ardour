// -*- c++ -*-
#ifndef _GLIBMM_ERROR_H
#define _GLIBMM_ERROR_H
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

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern "C" { typedef struct _GError GError; }
#endif

#include <glib/gquark.h>
#include <glibmm/exception.h>


namespace Glib
{

class Error : public Glib::Exception
{
public:
  Error();
  Error(GQuark domain, int code, const Glib::ustring& message);
  explicit Error(GError* gobject, bool take_copy = false);

  Error(const Error& other);
  Error& operator=(const Error& other);

  virtual ~Error() throw();

  GQuark domain() const;
  int code() const;
  virtual Glib::ustring what() const;

  bool matches(GQuark domain, int code) const;

  GError* gobj();
  const GError* gobj() const;

#ifndef DOXYGEN_SHOULD_SKIP_THIS

  void propagate(GError** dest);

  typedef void (* ThrowFunc) (GError*);

  static void register_init();
  static void register_cleanup();
  static void register_domain(GQuark domain, ThrowFunc throw_func);

  static void throw_exception(GError* gobject) G_GNUC_NORETURN;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

protected:
  GError* gobject_;
};

} // namespace Glib


#endif /* _GLIBMM_ERROR_H */

