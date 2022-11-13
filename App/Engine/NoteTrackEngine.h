#include "NoteSequence.h"
#include "TrackEngine.h"
#include "SequenceState.h"
#include "SortedQueue.h"

class NoteTrackEngine : public TrackEngine {
public:
    NoteTrackEngine(Engine &engine, const Model &model, NoteTrack &track) :
        TrackEngine(engine, model, track),
        _noteTrack(track)
    {
        reset();
    }

    virtual void reset() override;
    virtual bool tick(uint32_t tick) override;
    virtual void update(float dt) override;
    virtual void changePattern() override;

    virtual bool activity() const override { return _activity; }
    virtual bool gateOutput() const override { return _gateOutput; }
    virtual float cvOutput() const override { return _cvOutput; }

    virtual float sequenceProgress() const override {
        return _currentStep < 0 ? 0.f : float(_currentStep - _sequence->firstStep()) / (_sequence->lastStep() - _sequence->firstStep());
    }

    NoteSequence &sequence() const { return *_sequence; }

    int currentStep() const { return _currentStep; }

    inline bool stepTriggered() {
        bool result = _triggerStep;
        _triggerStep = false;
        return result;
    };

private:
    void triggerStep(uint32_t tick, uint32_t divisor);

    NoteTrack &_noteTrack;
    NoteSequence *_sequence;

    uint32_t _freeRelativeTick;
    SequenceState _sequenceState;
    int _currentStep;
    bool _triggerStep;

    bool _activity;
    bool _gateOutput;
    float _cvOutput;
    float _cvOutputTarget;

    struct Gate {
        uint32_t tick;
        bool gate;
    };

    struct GateCompare {
        bool operator()(const Gate &a, const Gate &b) {
            return a.tick < b.tick;
        }
    };

    SortedQueue<Gate, 16, GateCompare> _gateQueue;

    struct Cv {
        uint32_t tick;
        uint32_t cv;
    };

    struct CvCompare {
        bool operator()(const Cv &a, const Cv &b) {
            return a.tick < b.tick;
        }
    };

    SortedQueue<Cv, 16, CvCompare> _cvQueue;
};