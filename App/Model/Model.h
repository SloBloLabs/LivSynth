#pragma once

#include "Project.h"

class Model {
public:
    Model() {};
    void init();

    const Project &project() const { return _project; }
          Project &project()       { return _project; }
    
    // Settings...

private:
    Project _project;
    //Settings _settings;
};