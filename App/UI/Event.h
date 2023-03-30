#pragma once

#include "Key.h"

class KeyEvent {
public:
    enum Type {
        KeyUp,
        KeyDown
    };

    KeyEvent(Type type, const Key &key, int count, uint32_t duration) :
        _type(type),
        _key(key),
        _count(count),
        _duration(duration)
    {}

    inline Type type() const { return _type; }
    inline const Key &key() const { return _key; }
    inline int count() const { return _count; }
    inline uint32_t duration() const { return _duration; }
    inline bool isLong() const { return _duration > 2000; }

private:
    Type _type;
    Key _key;
    int _count;
    uint32_t _duration;

};

class PotEvent {
public:
    PotEvent(size_t index, float value) :
        _index(index),
        _value(value)
    {}

    inline size_t index() const { return _index; }
    inline float value() const { return _value; }

private:
    size_t _index;
    float _value;
};