// -*- c++ -*-
// Generated by gtkmmproc -- DO NOT MODIFY!
#ifndef _GTKMM_ENTRYCOMPLETION_P_H
#define _GTKMM_ENTRYCOMPLETION_P_H
#include <glibmm/private/object_p.h>

#include <glibmm/class.h>

namespace Gtk
{

class EntryCompletion_Class : public Glib::Class
{
public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef EntryCompletion CppObjectType;
  typedef GtkEntryCompletion BaseObjectType;
  typedef GtkEntryCompletionClass BaseClassType;
  typedef Glib::Object_Class CppClassParent;
  typedef GObjectClass BaseClassParent;

  friend class EntryCompletion;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  const Glib::Class& init();

  static void class_init_function(void* g_class, void* class_data);

  static Glib::ObjectBase* wrap_new(GObject*);

protected:

  //Callbacks (default signal handlers):
  //These will call the *_impl member methods, which will then call the existing default signal callbacks, if any.
  //You could prevent the original default signal handlers being called by overriding the *_impl method.
  static void action_activated_callback(GtkEntryCompletion* self, gint p0);
    static gboolean match_selected_callback_custom(GtkEntryCompletion* self, GtkTreeModel* c_model, GtkTreeIter* c_iter);
  
  //Callbacks (virtual functions):
};


} // namespace Gtk

#endif /* _GTKMM_ENTRYCOMPLETION_P_H */

