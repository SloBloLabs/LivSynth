#pragma once

#include "Key.h"

class KeyEvent {
public:
    enum Type {
        KeyUp,
        KeyDown
    };

    KeyEvent(Type type, const Key &key, int count) :
        _type(type),
        _key(key),
        _count(count)
    {}

    inline Type type() const { return _type; }
    inline const Key &key() const { return _key; }
    inline int count() const { return _count; }

private:
    Type _type;
    Key _key;
    int _count;

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