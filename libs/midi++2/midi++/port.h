/*
    Copyright (C) 1998-99 Paul Barton-Davis 
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

#ifndef  __libmidi_port_h__
#define  __libmidi_port_h__

#include <string>
#include <iostream>

#include <sigc++/sigc++.h>
#include <pbd/xml++.h>

#include <midi++/types.h>
#include <midi++/parser.h>

namespace MIDI {

class Channel;
class PortRequest;

class Port : public sigc::trackable {
  public:
	enum Type {
		Unknown,
		ALSA_RawMidi,
		ALSA_Sequencer,
		CoreMidi_MidiPort,
		Null,
		FIFO
	};


	Port (const XMLNode&);
	virtual ~Port ();

	virtual XMLNode& get_state () const;
	virtual void set_state (const XMLNode&);

	/* Direct I/O */

	virtual int write (byte *msg, size_t msglen) = 0;	
	virtual int read (byte *buf, size_t max) = 0;

	/* slowdown i/o to a loop of single byte emissions
	   interspersed with a busy loop of 10000 * this value.

	   This may be ignored by a particular instance
	   of this virtual class. See FD_MidiPort for an 
	   example of where it used.  
	*/

	void set_slowdown (size_t n) { slowdown = n; }

	/* select(2)/poll(2)-based I/O */

	virtual int selectable() const = 0;

	//void selector_read_callback (Select::Selectable *, Select::Condition);

	static void xforms_read_callback (int cond, int fd, void *ptr);
	static void gtk_read_callback (void *ptr, int fd, int cond);
	
	static void write_callback (byte *msg, unsigned int len, void *);
	
	Channel *channel (channel_t chn) { 
		return _channel[chn&0x7F];
	}
	
	Parser *input()     { return input_parser; }
	Parser *output()    { return output_parser; }

	void iostat (int *written, int *read, 
		     const size_t **in_counts,
		     const size_t **out_counts) {

		*written = bytes_written;
		*read = bytes_read;
		if (input_parser) {
			*in_counts = input_parser->message_counts();
		} else {
			*in_counts = 0;
		}
		if (output_parser) {
			*out_counts = output_parser->message_counts();
		} else {
			*out_counts = 0;
		}
	}
	
	int midimsg (byte *msg, size_t len) {
		return !(write (msg, len) == (int) len);
	} 

	int three_byte_msg (byte a, byte b, byte c) {
		byte msg[3];

            	msg[0] = a;
		msg[1] = b;
		msg[2] = c;

		return !(write (msg, 3) == 3);
	} 
	
	int clock ();
	
	const char *device () const { return _devname.c_str(); }
	const char *name () const   { return _tagname.c_str(); }
	Type   type () const        { return _type; }
	int    mode () const        { return _mode; }
	bool   ok ()   const        { return _ok; }

	struct Descriptor {
	    std::string tag;
	    std::string device;
	    int mode;
	    Port::Type type;

	    Descriptor (const XMLNode&);
	    XMLNode& get_state();
	};

  protected:
	bool _ok;
	Type _type;
	std::string _devname;
	std::string _tagname;
	int _mode;
	Channel *_channel[16];
	sigc::connection thru_connection;
	unsigned int bytes_written;
	unsigned int bytes_read;
	Parser *input_parser;
	Parser *output_parser;
	size_t slowdown;

	virtual std::string get_typestring () const = 0;

  private:
	static size_t nports;
};

struct PortSet {
    PortSet (std::string str) : owner (str) { }
    
    std::string owner;
    std::list<XMLNode> ports;
};

std::ostream & operator << ( std::ostream & os, const Port & port );

} // namespace MIDI

#endif // __libmidi_port_h__
