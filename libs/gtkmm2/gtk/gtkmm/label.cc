// Generated by gtkmmproc -- DO NOT MODIFY!

#include <gtkmm/label.h>
#include <gtkmm/private/label_p.h>

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

#include <gtk/gtklabel.h>
#include <gtkmm/menu.h>

namespace Gtk
{

Label::Label()
:
  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
  Gtk::Misc(Glib::ConstructParams(label_class_.init(), (char*) 0))
{}

Label::Label(const Glib::ustring& label, bool mnemonic)
:
  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
  Gtk::Misc(Glib::ConstructParams(label_class_.init(), "label",label.c_str(),"use_underline",gboolean(mnemonic), (char*) 0))
{}

Label::Label(const Glib::ustring& label, float xalign, float yalign, bool mnemonic)
:
  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
  Gtk::Misc(Glib::ConstructParams(label_class_.init(), "label",label.c_str(),"use_underline",gboolean(mnemonic), (char*) 0))
{
  set_alignment(xalign, yalign);
}

Label::Label(const Glib::ustring& label, AlignmentEnum xalign, AlignmentEnum yalign, bool mnemonic)
:
  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
  Gtk::Misc(Glib::ConstructParams(label_class_.init(), "label",label.c_str(),"use_underline",gboolean(mnemonic), (char*) 0))
{
  set_alignment(xalign, yalign);
}

void Label::select_region(int start_offset)
{
  gtk_label_select_region(gobj(), start_offset, -1 /* See C docs */);
}

} // namespace Gtk


namespace
{

void Label_signal_populate_popup_callback(GtkLabel* self, GtkMenu* p0,void* data)
{
  using namespace Gtk;
  typedef sigc::slot< void,Menu* > SlotType;

  // Do not try to call a signal on a disassociated wrapper.
  if(Glib::ObjectBase::_get_current_wrapper((GObject*) self))
  {
    try
    {
      if(sigc::slot_base *const slot = Glib::SignalProxyNormal::data_to_slot(data))
        (*static_cast<SlotType*>(slot))(Glib::wrap(p0)
);
    }
    catch(...)
    {
      Glib::exception_handlers_invoke();
    }
  }
}

const Glib::SignalProxyInfo Label_signal_populate_popup_info =
{
  "populate_popup",
  (GCallback) &Label_signal_populate_popup_callback,
  (GCallback) &Label_signal_populate_popup_callback
};

} // anonymous namespace


namespace Glib
{

Gtk::Label* wrap(GtkLabel* object, bool take_copy)
{
  return dynamic_cast<Gtk::Label *> (Glib::wrap_auto ((GObject*)(object), take_copy));
}

} /* namespace Glib */

namespace Gtk
{


/* The *_Class implementation: */

const Glib::Class& Label_Class::init()
{
  if(!gtype_) // create the GType if necessary
  {
    // Glib::Class has to know the class init function to clone custom types.
    class_init_func_ = &Label_Class::class_init_function;

    // This is actually just optimized away, apparently with no harm.
    // Make sure that the parent type has been created.
    //CppClassParent::CppObjectType::get_type();

    // Create the wrapper type, with the same class/instance size as the base type.
    register_derived_type(gtk_label_get_type());

    // Add derived versions of interfaces, if the C type implements any interfaces:
  }

  return *this;
}

void Label_Class::class_init_function(void* g_class, void* class_data)
{
  BaseClassType *const klass = static_cast<BaseClassType*>(g_class);
  CppClassParent::class_init_function(klass, class_data);

  klass->populate_popup = &populate_popup_callback;
}


void Label_Class::populate_popup_callback(GtkLabel* self, GtkMenu* p0)
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
      obj->on_populate_popup(Glib::wrap(p0)
);
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
    if(base && base->populate_popup)
      (*base->populate_popup)(self, p0);
  }
}


Glib::ObjectBase* Label_Class::wrap_new(GObject* o)
{
  return manage(new Label((GtkLabel*)(o)));

}


/* The implementation: */

Label::Label(const Glib::ConstructParams& construct_params)
:
  Gtk::Misc(construct_params)
{
  }

Label::Label(GtkLabel* castitem)
:
  Gtk::Misc((GtkMisc*)(castitem))
{
  }

Label::~Label()
{
  destroy_();
}

Label::CppClassType Label::label_class_; // initialize static member

GType Label::get_type()
{
  return label_class_.init().get_type();
}

GType Label::get_base_type()
{
  return gtk_label_get_type();
}


void Label::set_text(const Glib::ustring & str)
{
  gtk_label_set_text(gobj(), str.c_str());
}

Glib::ustring Label::get_text() const
{
  return Glib::convert_const_gchar_ptr_to_ustring(gtk_label_get_text(const_cast<GtkLabel*>(gobj())));
}

void Label::set_attributes(Pango::AttrList& attrs)
{
  gtk_label_set_attributes(gobj(), (attrs).gobj());
}

Pango::AttrList Label::get_attributes() const
{
  return Pango::AttrList((gtk_label_get_attributes(const_cast<GtkLabel*>(gobj()))));
}

void Label::set_label(const Glib::ustring& str)
{
  gtk_label_set_label(gobj(), str.c_str());
}

Glib::ustring Label::get_label() const
{
  return Glib::convert_const_gchar_ptr_to_ustring(gtk_label_get_label(const_cast<GtkLabel*>(gobj())));
}

void Label::set_markup(const Glib::ustring& str)
{
  gtk_label_set_markup(gobj(), str.c_str());
}

void Label::set_use_markup(bool setting)
{
  gtk_label_set_use_markup(gobj(), static_cast<int>(setting));
}

bool Label::get_use_markup() const
{
  return gtk_label_get_use_markup(const_cast<GtkLabel*>(gobj()));
}

void Label::set_use_underline(bool setting)
{
  gtk_label_set_use_underline(gobj(), static_cast<int>(setting));
}

bool Label::get_use_underline() const
{
  return gtk_label_get_use_underline(const_cast<GtkLabel*>(gobj()));
}

void Label::set_markup_with_mnemonic(const Glib::ustring& str)
{
  gtk_label_set_markup_with_mnemonic(gobj(), str.c_str());
}

guint Label::get_mnemonic_keyval() const
{
  return gtk_label_get_mnemonic_keyval(const_cast<GtkLabel*>(gobj()));
}

void Label::set_mnemonic_widget(Widget& widget)
{
  gtk_label_set_mnemonic_widget(gobj(), (widget).gobj());
}

Widget* Label::get_mnemonic_widget()
{
  return Glib::wrap(gtk_label_get_mnemonic_widget(gobj()));
}

const Widget* Label::get_mnemonic_widget() const
{
  return Glib::wrap(gtk_label_get_mnemonic_widget(const_cast<GtkLabel*>(gobj())));
}

void Label::set_text_with_mnemonic(const Glib::ustring& str)
{
  gtk_label_set_text_with_mnemonic(gobj(), str.c_str());
}

void Label::set_justify(Justification jtype)
{
  gtk_label_set_justify(gobj(), ((GtkJustification)(jtype)));
}

Justification Label::get_justify() const
{
  return ((Justification)(gtk_label_get_justify(const_cast<GtkLabel*>(gobj()))));
}

void Label::set_ellipsize(Pango::EllipsizeMode mode)
{
  gtk_label_set_ellipsize(gobj(), ((PangoEllipsizeMode)(mode)));
}

Pango::EllipsizeMode Label::get_ellipsize() const
{
  return ((Pango::EllipsizeMode)(gtk_label_get_ellipsize(const_cast<GtkLabel*>(gobj()))));
}

void Label::set_width_chars(int n_chars)
{
  gtk_label_set_width_chars(gobj(), n_chars);
}

int Label::get_width_chars() const
{
  return gtk_label_get_width_chars(const_cast<GtkLabel*>(gobj()));
}

void Label::set_max_width_chars(int n_chars)
{
  gtk_label_set_max_width_chars(gobj(), n_chars);
}

int Label::get_max_width_chars() const
{
  return gtk_label_get_max_width_chars(const_cast<GtkLabel*>(gobj()));
}

void Label::set_pattern(const Glib::ustring& pattern)
{
  gtk_label_set_pattern(gobj(), pattern.c_str());
}

void Label::set_line_wrap(bool wrap)
{
  gtk_label_set_line_wrap(gobj(), static_cast<int>(wrap));
}

bool Label::get_line_wrap() const
{
  return gtk_label_get_line_wrap(const_cast<GtkLabel*>(gobj()));
}

void Label::set_selectable(bool setting)
{
  gtk_label_set_selectable(gobj(), static_cast<int>(setting));
}

bool Label::get_selectable() const
{
  return gtk_label_get_selectable(const_cast<GtkLabel*>(gobj()));
}

void Label::set_angle(double angle)
{
  gtk_label_set_angle(gobj(), angle);
}

double Label::get_angle() const
{
  return gtk_label_get_angle(const_cast<GtkLabel*>(gobj()));
}

void Label::select_region(int start_offset, int end_offset)
{
  gtk_label_select_region(gobj(), start_offset, end_offset);
}

bool Label::get_selection_bounds(int& start, int& end) const
{
  return gtk_label_get_selection_bounds(const_cast<GtkLabel*>(gobj()), &start, &end);
}

Glib::RefPtr<Pango::Layout> Label::get_layout()
{

  Glib::RefPtr<Pango::Layout> retvalue = Glib::wrap(gtk_label_get_layout(gobj()));

  if(retvalue)
    retvalue->reference(); //The function does not do a ref for us.
  return retvalue;
}

Glib::RefPtr<const Pango::Layout> Label::get_layout() const
{

  Glib::RefPtr<const Pango::Layout> retvalue = Glib::wrap(gtk_label_get_layout(const_cast<GtkLabel*>(gobj())));

  if(retvalue)
    retvalue->reference(); //The function does not do a ref for us.
  return retvalue;
}

void Label::get_layout_offsets(int& x, int& y) const
{
  gtk_label_get_layout_offsets(const_cast<GtkLabel*>(gobj()), &x, &y);
}

void Label::set_single_line_mode(bool single_line_mode)
{
  gtk_label_set_single_line_mode(gobj(), static_cast<int>(single_line_mode));
}

bool Label::get_single_line_mode() const
{
  return gtk_label_get_single_line_mode(const_cast<GtkLabel*>(gobj()));
}


Glib::SignalProxy1< void,Menu* > Label::signal_populate_popup()
{
  return Glib::SignalProxy1< void,Menu* >(this, &Label_signal_populate_popup_info);
}


Glib::PropertyProxy<Glib::ustring> Label::property_label() 
{
  return Glib::PropertyProxy<Glib::ustring>(this, "label");
}

Glib::PropertyProxy_ReadOnly<Glib::ustring> Label::property_label() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "label");
}

Glib::PropertyProxy<Pango::AttrList> Label::property_attributes() 
{
  return Glib::PropertyProxy<Pango::AttrList>(this, "attributes");
}

Glib::PropertyProxy_ReadOnly<Pango::AttrList> Label::property_attributes() const
{
  return Glib::PropertyProxy_ReadOnly<Pango::AttrList>(this, "attributes");
}

Glib::PropertyProxy<bool> Label::property_use_markup() 
{
  return Glib::PropertyProxy<bool>(this, "use-markup");
}

Glib::PropertyProxy_ReadOnly<bool> Label::property_use_markup() const
{
  return Glib::PropertyProxy_ReadOnly<bool>(this, "use-markup");
}

Glib::PropertyProxy<bool> Label::property_use_underline() 
{
  return Glib::PropertyProxy<bool>(this, "use-underline");
}

Glib::PropertyProxy_ReadOnly<bool> Label::property_use_underline() const
{
  return Glib::PropertyProxy_ReadOnly<bool>(this, "use-underline");
}

Glib::PropertyProxy<Justification> Label::property_justify() 
{
  return Glib::PropertyProxy<Justification>(this, "justify");
}

Glib::PropertyProxy_ReadOnly<Justification> Label::property_justify() const
{
  return Glib::PropertyProxy_ReadOnly<Justification>(this, "justify");
}

Glib::PropertyProxy_WriteOnly<Glib::ustring> Label::property_pattern() 
{
  return Glib::PropertyProxy_WriteOnly<Glib::ustring>(this, "pattern");
}

Glib::PropertyProxy_ReadOnly<Glib::ustring> Label::property_pattern() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "pattern");
}

Glib::PropertyProxy<bool> Label::property_wrap() 
{
  return Glib::PropertyProxy<bool>(this, "wrap");
}

Glib::PropertyProxy_ReadOnly<bool> Label::property_wrap() const
{
  return Glib::PropertyProxy_ReadOnly<bool>(this, "wrap");
}

Glib::PropertyProxy<bool> Label::property_selectable() 
{
  return Glib::PropertyProxy<bool>(this, "selectable");
}

Glib::PropertyProxy_ReadOnly<bool> Label::property_selectable() const
{
  return Glib::PropertyProxy_ReadOnly<bool>(this, "selectable");
}

Glib::PropertyProxy_ReadOnly<guint> Label::property_mnemonic_keyval() const
{
  return Glib::PropertyProxy_ReadOnly<guint>(this, "mnemonic-keyval");
}

Glib::PropertyProxy<Widget*> Label::property_mnemonic_widget() 
{
  return Glib::PropertyProxy<Widget*>(this, "mnemonic-widget");
}

Glib::PropertyProxy_ReadOnly<Widget*> Label::property_mnemonic_widget() const
{
  return Glib::PropertyProxy_ReadOnly<Widget*>(this, "mnemonic-widget");
}

Glib::PropertyProxy_ReadOnly<int> Label::property_cursor_position() const
{
  return Glib::PropertyProxy_ReadOnly<int>(this, "cursor-position");
}

Glib::PropertyProxy_ReadOnly<int> Label::property_selection_bound() const
{
  return Glib::PropertyProxy_ReadOnly<int>(this, "selection-bound");
}

Glib::PropertyProxy<Pango::EllipsizeMode> Label::property_ellipsize() 
{
  return Glib::PropertyProxy<Pango::EllipsizeMode>(this, "ellipsize");
}

Glib::PropertyProxy_ReadOnly<Pango::EllipsizeMode> Label::property_ellipsize() const
{
  return Glib::PropertyProxy_ReadOnly<Pango::EllipsizeMode>(this, "ellipsize");
}

Glib::PropertyProxy<int> Label::property_width_chars() 
{
  return Glib::PropertyProxy<int>(this, "width-chars");
}

Glib::PropertyProxy_ReadOnly<int> Label::property_width_chars() const
{
  return Glib::PropertyProxy_ReadOnly<int>(this, "width-chars");
}

Glib::PropertyProxy<bool> Label::property_single_line_mode() 
{
  return Glib::PropertyProxy<bool>(this, "single-line-mode");
}

Glib::PropertyProxy_ReadOnly<bool> Label::property_single_line_mode() const
{
  return Glib::PropertyProxy_ReadOnly<bool>(this, "single-line-mode");
}

Glib::PropertyProxy<double> Label::property_angle() 
{
  return Glib::PropertyProxy<double>(this, "angle");
}

Glib::PropertyProxy_ReadOnly<double> Label::property_angle() const
{
  return Glib::PropertyProxy_ReadOnly<double>(this, "angle");
}

Glib::PropertyProxy<int> Label::property_max_width_chars() 
{
  return Glib::PropertyProxy<int>(this, "max-width-chars");
}

Glib::PropertyProxy_ReadOnly<int> Label::property_max_width_chars() const
{
  return Glib::PropertyProxy_ReadOnly<int>(this, "max-width-chars");
}


void Gtk::Label::on_populate_popup(Menu* menu)
{
  BaseClassType *const base = static_cast<BaseClassType*>(
      g_type_class_peek_parent(G_OBJECT_GET_CLASS(gobject_)) // Get the parent class of the object class (The original underlying C class).
  );

  if(base && base->populate_popup)
    (*base->populate_popup)(gobj(),(GtkMenu*)Glib::unwrap(menu));
}


} // namespace Gtk


