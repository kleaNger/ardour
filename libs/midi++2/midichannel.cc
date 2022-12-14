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

    $Id$
*/

#include <cstring>
#include <midi++/types.h>
#include <midi++/port.h>
#include <midi++/channel.h>

using namespace sigc;
using namespace MIDI;

Channel::Channel (byte channelnum, Port &p) : port (p)
{
	channel_number = channelnum;

	reset (false);
}	

void
Channel::connect_input_signals ()

{
	port.input()->channel_pressure[channel_number].connect
		(mem_fun (*this, &Channel::process_chanpress));
	port.input()->channel_note_on[channel_number].connect
		(mem_fun (*this, &Channel::process_note_on));
	port.input()->channel_note_off[channel_number].connect
		(mem_fun (*this, &Channel::process_note_off));
	port.input()->channel_poly_pressure[channel_number].connect
		(mem_fun (*this, &Channel::process_polypress));
	port.input()->channel_program_change[channel_number].connect
		(mem_fun (*this, &Channel::process_program_change));
	port.input()->channel_controller[channel_number].connect
		(mem_fun (*this, &Channel::process_controller));
	port.input()->channel_pitchbend[channel_number].connect
		(mem_fun (*this, &Channel::process_pitchbend));
	port.input()->reset.connect (mem_fun (*this, &Channel::process_reset));
}

void
Channel::connect_output_signals ()

{
	port.output()->channel_pressure[channel_number].connect
		(mem_fun (*this, &Channel::process_chanpress));
	port.output()->channel_note_on[channel_number].connect
		(mem_fun (*this, &Channel::process_note_on));
	port.output()->channel_note_off[channel_number].connect
		(mem_fun (*this, &Channel::process_note_off));
	port.output()->channel_poly_pressure[channel_number].connect
		(mem_fun (*this, &Channel::process_polypress));
	port.output()->channel_program_change[channel_number].connect
		(mem_fun (*this, &Channel::process_program_change));
	port.output()->channel_controller[channel_number].connect
		(mem_fun (*this, &Channel::process_controller));
	port.output()->channel_pitchbend[channel_number].connect
		(mem_fun (*this, &Channel::process_pitchbend));
	port.output()->reset.connect (mem_fun (*this, &Channel::process_reset));
}

void
Channel::reset (bool notes_off)
{
	program_number = channel_number;
	bank_number = 0;
	pitch_bend = 0;

	_last_note_on = 0;
	_last_note_off = 0;
	_last_on_velocity = 0;
	_last_off_velocity = 0;

	if (notes_off) {
		all_notes_off ();
	}

	memset (polypress, 0, sizeof (polypress));
	memset (controller_msb, 0, sizeof (controller_msb));
	memset (controller_lsb, 0, sizeof (controller_lsb));
       
	/* zero all controllers XXX not necessarily the right thing */

	memset (controller_val, 0, sizeof (controller_val));

	for (int n = 0; n < 128; n++) {
		controller_14bit[n] = false;
	}

	rpn_msb = 0;
	rpn_lsb = 0;
	nrpn_msb = 0;
	nrpn_lsb = 0;

	_omni = true;
	_poly = false;
	_mono = true;
	_notes_on = 0;
}

void
Channel::process_note_off (Parser &parser, EventTwoBytes *tb) 

{
	_last_note_off = tb->note_number;
	_last_off_velocity = tb->velocity;

	if (_notes_on) {
		_notes_on--;
	}
}

void
Channel::process_note_on (Parser &parser, EventTwoBytes *tb) 

{
	_last_note_on = tb->note_number;
	_last_on_velocity = tb->velocity;
	_notes_on++;
}

void
Channel::process_controller (Parser &parser, EventTwoBytes *tb) 

{
	unsigned short cv;

	/* XXX arguably need a lock here to protect non-atomic changes
	   to controller_val[...]. or rather, need to make sure that
	   all changes *are* atomic.
	*/

	if (tb->controller_number <= 31) { /* unsigned: no test for >= 0 */

		/* if this controller is already known to use 14 bits,
		   then treat this value as the MSB, and combine it 
		   with the existing LSB.

		   otherwise, just treat it as a 7 bit value, and set
		   it directly.
		*/

		cv = (unsigned short) controller_val[tb->controller_number];

		if (controller_14bit[tb->controller_number]) {
			cv = ((tb->value << 7) | (cv & 0x7f));
		} else {
			cv = tb->value;
		}

		controller_val[tb->controller_number] = (controller_value_t)cv;

	} else if ((tb->controller_number >= 32 && 
		    tb->controller_number <= 63)) {
		   
		cv = (unsigned short) controller_val[tb->controller_number];

		/* LSB for CC 0-31 arrived. 

		   If this is the first time (i.e. its currently
		   flagged as a 7 bit controller), mark the
		   controller as 14 bit, adjust the existing value
		   to be the MSB, and OR-in the new LSB value. 

		   otherwise, OR-in the new low 7bits with the old
		   high 7.
		*/

		int cn = tb->controller_number - 32;
		   
		if (controller_14bit[cn] == false) {
			controller_14bit[cn] = true;
			cv = (cv << 7) | (tb->value & 0x7f);
		} else {
			cv = (cv & 0x3f80) | (tb->value & 0x7f);
		}

		controller_val[tb->controller_number] = 
			(controller_value_t) cv;
	} else {

		/* controller can only take 7 bit values */
		
		controller_val[tb->controller_number] = 
			(controller_value_t) tb->value;
	}

	/* bank numbers are special, in that they have their own signal
	 */

	if (tb->controller_number == 0) {
		bank_number = (unsigned short) controller_val[0];
		if (port.input()) {
			port.input()->bank_change (*port.input(), bank_number);
			port.input()->channel_bank_change[channel_number] 
				(*port.input(), bank_number);
		}
	}

}

void
Channel::process_program_change (Parser &parser, byte val) 

{
	program_number = val;
}

void
Channel::process_chanpress (Parser &parser, byte val) 

{
	chanpress = val;
}

void
Channel::process_polypress (Parser &parser, EventTwoBytes *tb) 

{
	polypress[tb->note_number] = tb->value;
}

void
Channel::process_pitchbend (Parser &parser, pitchbend_t val) 

{
	pitch_bend = val;
}

void
Channel::process_reset (Parser &parser) 

{
	reset ();
}

int
Channel::channel_msg (byte id, byte val1, byte val2)

{
	unsigned char msg[3];
	int len = 0;

	msg[0] = id | (channel_number & 0xf);

	switch (id) {
	case off:
		msg[1] = val1 & 0x7F;
		msg[2] = val2 & 0x7F;
		len = 3;
		break;

	case on:
		msg[1] = val1 & 0x7F;
		msg[2] = val2 & 0x7F;
		len = 3;
		break;

	case MIDI::polypress:
		msg[1] = val1 & 0x7F;
		msg[2] = val2 & 0x7F;
		len = 3;
		break;

	case controller:
		msg[1] = val1 & 0x7F;
		msg[2] = val2 & 0x7F;
		len = 3;
		break;

	case MIDI::program:
		msg[1] = val1 & 0x7F;
		len = 2;
		break;

	case MIDI::chanpress:
		msg[1] = val1 & 0x7F;
		len = 2;
		break;

	case MIDI::pitchbend:
		msg[1] = val1 & 0x7F;
		msg[2] = val2 & 0x7F;
		len = 3;
		break;
	}

	return port.midimsg (msg, len);
}
