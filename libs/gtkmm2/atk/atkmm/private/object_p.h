// -*- c++ -*-
// Generated by gtkmmproc -- DO NOT MODIFY!
#ifndef _ATKMM_OBJECT_P_H
#define _ATKMM_OBJECT_P_H
#include <glibmm/private/object_p.h>

#include <glibmm/class.h>

namespace Atk
{

class Object_Class : public Glib::Class
{
public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef Object CppObjectType;
  typedef AtkObject BaseObjectType;
  typedef AtkObjectClass BaseClassType;
  typedef Glib::Object_Class CppClassParent;
  typedef GObjectClass BaseClassParent;

  friend class Object;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  const Glib::Class& init();

  static void class_init_function(void* g_class, void* class_data);

  static Glib::ObjectBase* wrap_new(GObject*);

protected:

  //Callbacks (default signal handlers):
  //These will call the *_impl member methods, which will then call the existing default signal callbacks, if any.
  //You could prevent the original default signal handlers being called by overriding the *_impl method.
  static void children_changed_callback(AtkObject* self, guint p0, gpointer p1);
  static void focus_event_callback(AtkObject* self, gboolean p0);
  static void property_change_callback(AtkObject* self, AtkPropertyValues* p0);
  static void state_change_callback(AtkObject* self, const gchar* p0, gboolean p1);
  static void visible_data_changed_callback(AtkObject* self);
  static void active_descendant_changed_callback(AtkObject* self, gpointer* p0);

  //Callbacks (virtual functions):
};


} // namespace Atk

#endif /* _ATKMM_OBJECT_P_H */

