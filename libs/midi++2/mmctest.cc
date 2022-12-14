#include <cstdio>
#include <fcntl.h>

#include <pbd/error.h>
#include <pbd/textreceiver.h>

Transmitter error (Transmitter::Error);
Transmitter info (Transmitter::Info);
Transmitter warning (Transmitter::Warning);
Transmitter fatal (Transmitter::Fatal);
TextReceiver text_receiver ("mmctest");

#include "midi++/port.h"
#include "midi++/manager.h"
#include "midi++/mmc.h"

using namespace MIDI;
using namespace PBD;

Port *port;
PortRequest midi_device;
Parser *parser;
MachineControl *mmc;
MachineControl::CommandSignature cs;
MachineControl::ResponseSignature rs;

int 
setup_midi ()

{
	midi_device.devname = "/dev/snd/midiC0D0";
	midi_device.tagname = "trident";
	midi_device.mode = O_RDWR;
	midi_device.type = Port::ALSA_RawMidi;

	if ((port = MIDI::Manager::instance()->add_port (midi_device)) == 0) {
		info << "MIDI port is not valid" << endmsg;
		return -1;
	} 

	mmc = new MachineControl (*port, 0.0, cs, rs);

	return 0;
}

void
do_deferred_play (MachineControl &mmc)

{
	cout << "Deferred Play" << endl;
}

void
do_stop (MachineControl &mmc)

{
	cout << "Stop" << endl;
}

void
do_ffwd (MachineControl &mmc)

{
	cout << "Fast Forward" << endl;
}

void
do_rewind (MachineControl &mmc)

{
	cout << "Rewind" << endl;
}

void
do_record_status (MachineControl &mmc, size_t track, bool enabled)

{
	cout << "Track " << track + 1 << (enabled ? " enabled" : " disabled")
	     << endl;
}

main (int argc, char *argv[])

{
	byte buf[1];
	
	text_receiver.listen_to (error);
	text_receiver.listen_to (info);
	text_receiver.listen_to (fatal);
	text_receiver.listen_to (warning);

	if (setup_midi ()) {
		exit (1);
	}


	mmc->DeferredPlay.connect (mem_fun (do_deferred_play));
	mmc->FastForward.connect (mem_fun (do_ffwd));
	mmc->Rewind.connect (mem_fun (do_rewind));
	mmc->Stop.connect (mem_fun (do_stop));
	mmc->TrackRecordStatusChange.connect (mem_fun (do_record_status));

	while (1) {
		if (port->read (buf, 1) < 0) {
			error << "cannot read byte"
			      << endmsg;
			break;
		} 
	}
}


