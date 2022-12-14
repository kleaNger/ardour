// Generated by gtkmmproc -- DO NOT MODIFY!

#include <atkmm/implementor.h>
#include <atkmm/private/implementor_p.h>

// -*- c++ -*-
/* $Id$ */

/* Copyright 1998-2002 The gtkmm Development Team
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

#include <atkmm/object.h>
#include <atk/atkobject.h>


namespace
{
} // anonymous namespace


namespace Glib
{

Glib::RefPtr<Atk::Implementor> wrap(AtkImplementor* object, bool take_copy)
{
  return Glib::RefPtr<Atk::Implementor>( dynamic_cast<Atk::Implementor*> (Glib::wrap_auto ((GObject*)(object), take_copy)) );
  //We use dynamic_cast<> in case of multiple inheritance.
}

} // namespace Glib


namespace Atk
{


/* The *_Class implementation: */

const Glib::Interface_Class& Implementor_Class::init()
{
  if(!gtype_) // create the GType if necessary
  {
    // Glib::Interface_Class has to know the interface init function
    // in order to add interfaces to implementing types.
    class_init_func_ = &Implementor_Class::iface_init_function;

    // We can not derive from another interface, and it is not necessary anyway.
    gtype_ = atk_implementor_get_type();
  }

  return *this;
}

void Implementor_Class::iface_init_function(void* g_iface, void*)
{
  BaseClassType *const klass = static_cast<BaseClassType*>(g_iface);

  //This is just to avoid an "unused variable" warning when there are no vfuncs or signal handlers to connect.
  //This is a temporary fix until I find out why I can not seem to derive a GtkFileChooser interface. murrayc
  g_assert(klass != 0); 

  klass->ref_accessible = &ref_accessible_vfunc_callback;
}

AtkObject* Implementor_Class::ref_accessible_vfunc_callback(AtkImplementor* self)
{
  CppObjectType *const obj = dynamic_cast<CppObjectType*>(
      Glib::ObjectBase::_get_current_wrapper((GObject*)self));

  // Non-gtkmmproc-generated custom classes implicitly call the default
  // Glib::ObjectBase constructor, which sets is_derived_. But gtkmmproc-
  // generated classes can use this optimisation, which avoids the unnecessary
  // parameter conversions if there is no possibility of the virtual function
  // being overridden:
  if(obj && obj->is_derived_())
  {
    try // Trap C++ exceptions which would normally be lost because this is a C callback.
    {
      // Call the virtual member method, which derived classes might override.
      return Glib::unwrap(obj->ref_accessibile_vfunc());
    }
    catch(...)
    {
      Glib::exception_handlers_invoke();
    }
  }
  else
  {
    BaseClassType *const base = static_cast<BaseClassType*>(
        g_type_interface_peek_parent( // Get the parent interface of the interface (The original underlying C interface).
g_type_interface_peek(G_OBJECT_GET_CLASS(self), CppObjectType::get_type()) // Get the interface.
)    );

    // Call the original underlying C function:
    if(base && base->ref_accessible)
      return (*base->ref_accessible)(self);
  }

  typedef AtkObject* RType;
  return RType();
}


Glib::ObjectBase* Implementor_Class::wrap_new(GObject* object)
{
  return new Implementor((AtkImplementor*)(object));
}


/* The implementation: */

Implementor::Implementor()
:
  Glib::Interface(implementor_class_.init())
{}

Implementor::Implementor(AtkImplementor* castitem)
:
  Glib::Interface((GObject*)(castitem))
{}

Implementor::~Implementor()
{}

// static
void Implementor::add_interface(GType gtype_implementer)
{
  implementor_class_.init().add_interface(gtype_implementer);
}

Implementor::CppClassType Implementor::implementor_class_; // initialize static member

GType Implementor::get_type()
{
  return implementor_class_.init().get_type();
}

GType Implementor::get_base_type()
{
  return atk_implementor_get_type();
}


Glib::RefPtr<Object> Atk::Implementor::ref_accessibile_vfunc() 
{
  BaseClassType *const base = static_cast<BaseClassType*>(
      g_type_interface_peek_parent( // Get the parent interface of the interface (The original underlying C interface).
g_type_interface_peek(G_OBJECT_GET_CLASS(gobject_), CppObjectType::get_type()) // Get the interface.
)  );

  if(base && base->ref_accessible)
    return Glib::wrap((*base->ref_accessible)(gobj()));

  typedef Glib::RefPtr<Object> RType;
  return RType();
}


} // namespace Atk


