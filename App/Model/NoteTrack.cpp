#include "NoteTrack.h"

void NoteTrack::clear() {
    for(auto &sequence : _sequences) {
        sequence.clear();
    }
}