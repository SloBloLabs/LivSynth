#pragma once

#include "Config.h"
#include "NoteSequence.h"
#include <array>

class NoteTrack {
public:
    typedef std::array<NoteSequence, CONFIG_PATTERN_COUNT> NoteSequenceArray;

    const NoteSequenceArray &sequences() const { return _sequences; }
          NoteSequenceArray &sequences()       { return _sequences; }
    
    const NoteSequence &sequence(int index) const { return _sequences[index]; }
          NoteSequence &sequence(int index)       { return _sequences[index]; }
    
    NoteTrack() { clear(); }

    void clear();

private:
    NoteSequenceArray _sequences;
};