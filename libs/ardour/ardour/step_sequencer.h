/*
 * Copyright (C) 2018 Paul Davis <paul@linuxaudiosystems.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __libardour_step_sequencer_h__
#define __libardour_step_sequencer_h__

#include <vector>
#include <unistd.h>

#include <glibmm/threads.h>

#include "temporal/types.h"
#include "temporal/beats.h"

#include "ardour/mode.h"
#include "ardour/types.h"

namespace ARDOUR {

class MidiBuffer;
class MidiStateTracker;
class StepSequencer;
class StepSequence;
class TempoMap;

typedef std::pair<Temporal::Beats,samplepos_t> BeatPosition;
typedef std::vector<BeatPosition> BeatPositions;

class Step {
  public:
	enum Mode {
		AbsolutePitch,
		RelativePitch
	};

	Step (StepSequence&, Temporal::Beats const & beat);
	~Step ();

	void set_note (double note, double velocity = 0.5, int32_t duration = 1, int n = 0);
	void set_chord (size_t note_cnt, double* notes);
	void set_parameter (int number, double value, int n = 0);

	Mode mode() const { return _mode; }
	void set_mode (Mode m);

	double note (size_t n = 0) const { return _notes[n].number; }
	double velocity (size_t n = 0) const { return _notes[n].velocity; }
	int32_t duration (size_t n = 0) const { return _notes[n].duration; }

	void set_offset (Temporal::Beats const &, size_t n = 0);
	Temporal::Beats offset (size_t n = 0) const { return _notes[n].offset; }

	int parameter (size_t n = 0) const { return _parameters[n].parameter; }
	int parameter_value (size_t n = 0) const { return _parameters[n].value; }

	void set_enabled (bool);
	bool enabled() const { return _enabled; }

	void set_repeat (size_t r);
	size_t repeat() const { return _repeat; }

	void set_beat (Temporal::Beats const & beat);
	Temporal::Beats beat () const { return _nominal_beat; }

	bool run (MidiBuffer& buf, bool running, samplepos_t, samplepos_t, MidiStateTracker&);

	bool skipped() const { return _skipped; }
	void set_skipped (bool);

	void set_timeline_offset (Temporal::Beats const &, Temporal::Beats const &);

  private:
	StepSequence&      _sequence;
	bool               _enabled;
	Temporal::Beats     timeline_offset;
	Temporal::Beats    _nominal_beat;
	Temporal::Beats    _scheduled_beat;
	bool               _skipped;
	Mode               _mode;

	struct ParameterValue {
		int parameter;
		double value;
	};

	struct Note {
		union {
			double number;   /* typically MIDI note number */
			double interval; /* semitones */
		};
		double velocity;
		int32_t duration;
		Temporal::Beats offset;
		bool on;
		Temporal::Beats off_at;

		Note () : number (-1), velocity (0.5), duration (1), on (false) {}
		Note (double n, double v, double d, Temporal::Beats const & o) : number (n), velocity (v), duration (d), offset (o), on (false) {}
	};

	static const int _notes_per_step = 5;
	static const int _parameters_per_step = 5;

	Note _notes[_notes_per_step];
	ParameterValue _parameters[_parameters_per_step];
	size_t _repeat;

	void check_note (size_t n, MidiBuffer& buf, bool, samplepos_t, samplepos_t, MidiStateTracker&);
	void check_parameter (size_t n, MidiBuffer& buf, bool, samplepos_t, samplepos_t);


	StepSequencer& sequencer() const;
};

class StepSequence
{
  public:
	enum Direction {
		forwards = 0,
		backwards = 1,
		end_to_end = 2,
		rd_random = 3
	};

	StepSequence (StepSequencer &myseq, size_t nsteps, Temporal::Beats const & step_size, Temporal::Beats const & bar_size);
	~StepSequence ();

	void startup (Temporal::Beats const & start, Temporal::Beats const & offset);

	Temporal::Beats bar_size() const { return _bar_size; }

	double root() const { return _root; }
	void set_root (double n);

	int channel() const { return _channel; }
	void set_channel (int);

	Temporal::Beats wrap (Temporal::Beats const &) const;

	MusicalMode mode() const { return _mode; }
	void set_mode (MusicalMode m);

	void shift_left (size_t n = 1);
	void shift_right (size_t n = 1);

	void set_start_step (size_t);
	void set_end_step (size_t);
	void set_start_and_end_step (size_t, size_t);

	void set_step_size (Temporal::Beats const &);
	Temporal::Beats step_size () const { return _step_size; }

	void reset ();

	bool run (MidiBuffer& buf, bool running, samplepos_t, samplepos_t, MidiStateTracker&);

	StepSequencer& sequencer() const { return _sequencer; }

  private:
	StepSequencer& _sequencer;
	Glib::Threads::Mutex _step_lock;
	typedef std::vector<Step*> Steps;

	Steps       _steps;
	size_t      _start;   /* step count */
	size_t      _end;     /* step count */
	int         _channel; /* MIDI channel */

	Temporal::Beats _step_size;
	Temporal::Beats _bar_size;
	Temporal::Beats end_beat;

	double      _root;
	MusicalMode _mode;
};

class StepSequencer {
  public:
	StepSequencer (TempoMap&, size_t nseqs, size_t nsteps, Temporal::Beats const & step_size, Temporal::Beats const & bar_size);
	~StepSequencer ();

	Temporal::Beats duration() const;

	void startup (Temporal::Beats const & start, Temporal::Beats const & offset);

	Temporal::Beats step_size () const { return _step_size; }
	void set_step_size (Temporal::Beats const &);

	void set_start_step (size_t);
	void set_end_step (size_t);
	void set_start_and_end_step (size_t, size_t);

	void sync ();        /* return all rows to start step */
	void reset ();       /* return entire state to default */

	bool run (MidiBuffer& buf, bool running, samplepos_t, samplepos_t, MidiStateTracker&);

	TempoMap& tempo_map() const { return _tempo_map; }

  private:
	Glib::Threads::Mutex       _sequence_lock;

	typedef std::vector<StepSequence*> StepSequences;

	StepSequences  _sequences;

	TempoMap&       _tempo_map;
	Temporal::Beats _step_size;
	int32_t         _start;
	int32_t         _end;
};

} /* namespace */

#endif /* __libardour_step_sequencer_h__ */