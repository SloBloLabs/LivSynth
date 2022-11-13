#include "Project.h"

Project::Project() {
    clear();
}

void Project::clear() {
    // TODO: define default project values
    setTempo(20.f);
    setSwing(50);
    setTimeSignature(TimeSignature());
    _clockSetup.clear();
    _track.clear();

    noteSequence(0).setGates({ 1,0,0,1,0,1,0,0 });
    noteSequence(0).setNotes({ 2720, 2, 999, 3944, 3321, 1768, 3456, 22, 666 });
}