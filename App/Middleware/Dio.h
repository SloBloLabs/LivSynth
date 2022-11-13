#pragma once

class Dio {
public:
    void setClock(bool clock);
    void setReset(bool reset);
    void setGate(bool gate);
    
    void update();
private:
    bool _clock = false;
    bool _reset = false;
    bool _gate = false;
};