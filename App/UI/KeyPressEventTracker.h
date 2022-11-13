#include "Event.h"

#include "System.h"

class KeyPressEventTracker {
public:
    KeyEvent process(const Key &key) {
        uint32_t currentTicks = System::ticks();
        uint32_t deltaTicks = currentTicks - _lastTicks;

        if(key.code() != _lastCode || deltaTicks > 300) {
            _count = 1;
        } else {
            ++_count;
        }

        _lastCode = key.code();
        _lastTicks = currentTicks;

        return KeyEvent(KeyEvent::Type::KeyDown, key, _count);
    }

private:
    uint8_t _lastCode = Key::None;
    uint32_t _lastTicks = 0;
    uint8_t _count = 1;
};