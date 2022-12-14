/*
    Copyright (C) 2006 Andre Raue

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef __export_region_dialog_h__
#define __export_region_dialog_h__

#include "ardour_ui.h"
#include "export_dialog.h"


class ExportRegionDialog : public ExportDialog
{
  public:
	ExportRegionDialog (PublicEditor&, boost::shared_ptr<ARDOUR::Region>);
 
 	static void* _export_region_thread (void *);
	void export_region ();

  protected:
	void export_audio_data();
  
  private:
	boost::shared_ptr<ARDOUR::AudioRegion> audio_region;
};


#endif // __export_region_dialog_h__
