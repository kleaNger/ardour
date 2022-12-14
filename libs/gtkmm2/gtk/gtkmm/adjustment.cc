// Generated by gtkmmproc -- DO NOT MODIFY!

#include <gtkmm/adjustment.h>
#include <gtkmm/private/adjustment_p.h>

// -*- c++ -*-
/* $Id$ */

/* 
 *
 * Copyright 1998-2002 The gtkmm Development Team
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
#include <gtk/gtkadjustment.h>

namespace Gtk
{

Adjustment::Adjustment(double value,
      	               double lower, double upper,
      	               double step_increment, double page_increment,
      	               double page_size)
:
  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
  Gtk::Object(Glib::ConstructParams(adjustment_class_.init(), (char*) 0))
{
  gobj()->lower = lower;
  gobj()->upper = upper;
  gobj()->step_increment = step_increment;
  gobj()->page_increment = page_increment;
  gobj()->page_size = page_size;
  changed();

  set_value(value);
}

void Adjustment::set_lower(double lower)
{
  gobj()->lower = lower;
  changed();
}

void Adjustment::set_upper(double upper)
{
  gobj()->upper = upper;
  changed();
}

void Adjustment::set_step_increment(double incr)
{
  gobj()->step_increment = incr;
  changed();
}

void Adjustment::set_page_increment(double incr)
{
  gobj()->page_increment = incr;
  changed();
}

void Adjustment::set_page_size(double size)
{
  gobj()->page_size = size;
  changed();
}

} // namespace Gtk


namespace
{

const Glib::SignalProxyInfo Adjustment_signal_changed_info =
{
  "changed",
  (GCallback) &Glib::SignalProxyNormal::slot0_void_callback,
  (GCallback) &Glib::SignalProxyNormal::slot0_void_callback
};


const Glib::SignalProxyInfo Adjustment_signal_value_changed_info =
{
  "value_changed",
  (GCallback) &Glib::SignalProxyNormal::slot0_void_callback,
  (GCallback) &Glib::SignalProxyNormal::slot0_void_callback
};

} // anonymous namespace


namespace Glib
{

Gtk::Adjustment* wrap(GtkAdjustment* object, bool take_copy)
{
  return dynamic_cast<Gtk::Adjustment *> (Glib::wrap_auto ((GObject*)(object), take_copy));
}

} /* namespace Glib */

namespace Gtk
{


/* The *_Class implementation: */

const Glib::Class& Adjustment_Class::init()
{
  if(!gtype_) // create the GType if necessary
  {
    // Glib::Class has to know the class init function to clone custom types.
    class_init_func_ = &Adjustment_Class::class_init_function;

    // This is actually just optimized away, apparently with no harm.
    // Make sure that the parent type has been created.
    //CppClassParent::CppObjectType::get_type();

    // Create the wrapper type, with the same class/instance size as the base type.
    register_derived_type(gtk_adjustment_get_type());

    // Add derived versions of interfaces, if the C type implements any interfaces:
  }

  return *this;
}

void Adjustment_Class::class_init_function(void* g_class, void* class_data)
{
  BaseClassType *const klass = static_cast<BaseClassType*>(g_class);
  CppClassParent::class_init_function(klass, class_data);

  klass->changed = &changed_callback;
  klass->value_changed = &value_changed_callback;
}


void Adjustment_Class::changed_callback(GtkAdjustment* self)
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
      obj->on_changed();
    }
    catch(...)
    {
      Glib::exception_handlers_invoke();
    }
  }
  else
  {
    BaseClassType *const base = static_cast<BaseClassType*>(
        g_type_class_peek_parent(G_OBJECT_GET_CLASS(self)) // Get the parent class of the object class (The original underlying C class).
    );

    // Call the original underlying C function:
    if(base && base->changed)
      (*base->changed)(self);
  }
}

void Adjustment_Class::value_changed_callback(GtkAdjustment* self)
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
      obj->on_value_changed();
    }
    catch(...)
    {
      Glib::exception_handlers_invoke();
    }
  }
  else
  {
    BaseClassType *const base = static_cast<BaseClassType*>(
        g_type_class_peek_parent(G_OBJECT_GET_CLASS(self)) // Get the parent class of the object class (The original underlying C class).
    );

    // Call the original underlying C function:
    if(base && base->value_changed)
      (*base->value_changed)(self);
  }
}


Glib::ObjectBase* Adjustment_Class::wrap_new(GObject* o)
{
  return manage(new Adjustment((GtkAdjustment*)(o)));

}


/* The implementation: */

Adjustment::Adjustment(const Glib::ConstructParams& construct_params)
:
  Gtk::Object(construct_params)
{
  }

Adjustment::Adjustment(GtkAdjustment* castitem)
:
  Gtk::Object((GtkObject*)(castitem))
{
  }

Adjustment::~Adjustment()
{
  destroy_();
}

Adjustment::CppClassType Adjustment::adjustment_class_; // initialize static member

GType Adjustment::get_type()
{
  return adjustment_class_.init().get_type();
}

GType Adjustment::get_base_type()
{
  return gtk_adjustment_get_type();
}


void Adjustment::changed()
{
  gtk_adjustment_changed(gobj());
}

void Adjustment::value_changed()
{
  gtk_adjustment_value_changed(gobj());
}

void Adjustment::clamp_page(double lower, double upper)
{
  gtk_adjustment_clamp_page(gobj(), lower, upper);
}

void Adjustment::set_value(double value)
{
  gtk_adjustment_set_value(gobj(), value);
}

double Adjustment::get_value() const
{
  return gtk_adjustment_get_value(const_cast<GtkAdjustment*>(gobj()));
}

double Adjustment::get_lower() const
{
  return gobj()->lower;
}

double Adjustment::get_upper() const
{
  return gobj()->upper;
}

double Adjustment::get_step_increment() const
{
  return gobj()->step_increment;
}

double Adjustment::get_page_increment() const
{
  return gobj()->page_increment;
}

double Adjustment::get_page_size() const
{
  return gobj()->page_size;
}


Glib::SignalProxy0< void > Adjustment::signal_changed()
{
  return Glib::SignalProxy0< void >(this, &Adjustment_signal_changed_info);
}

Glib::SignalProxy0< void > Adjustment::signal_value_changed()
{
  return Glib::SignalProxy0< void >(this, &Adjustment_signal_value_changed_info);
}


void Gtk::Adjustment::on_changed()
{
  BaseClassType *const base = static_cast<BaseClassType*>(
      g_type_class_peek_parent(G_OBJECT_GET_CLASS(gobject_)) // Get the parent class of the object class (The original underlying C class).
  );

  if(base && base->changed)
    (*base->changed)(gobj());
}

void Gtk::Adjustment::on_value_changed()
{
  BaseClassType *const base = static_cast<BaseClassType*>(
      g_type_class_peek_parent(G_OBJECT_GET_CLASS(gobject_)) // Get the parent class of the object class (The original underlying C class).
  );

  if(base && base->value_changed)
    (*base->value_changed)(gobj());
}


} // namespace Gtk


