#pragma once

class Dio {
public:
    void init();
    void setClock(bool clock);
    void setReset(bool reset);
    inline bool gate() const { return _gate; }
    void setGate(bool gate);
    
    void update();
private:
    bool _clock;
    bool _reset;
    bool _gate;
};