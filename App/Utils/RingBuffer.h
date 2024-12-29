#pragma once

#include <cstddef>
#include <cstdint>
#include <atomic>

template<typename T, int32_t Size>
class RingBuffer {
public:
    constexpr RingBuffer() : _readPtr(0), _writePtr(0) {}
    
    inline void init() {
        _readPtr.store(0, std::memory_order_relaxed);
        _writePtr.store(0, std::memory_order_relaxed);
    }

    inline size_t size() const {
        return Size;
    }

    inline bool empty() const {
        return _readPtr.load(std::memory_order_acquire) == _writePtr.load(std::memory_order_acquire);
    }

    inline bool full() const {
        return writable() == 0;
    }

    inline size_t entries() const {
        int32_t readPtr = _readPtr.load(std::memory_order_acquire);
        int32_t writePtr = _writePtr.load(std::memory_order_acquire);
        int32_t size = (writePtr - readPtr) % Size;
        if(size < 0) size += Size;
        return static_cast<size_t>(size);
    }

    inline size_t writable() const {
        return Size - readable() - 1;
    }

    inline size_t readable() const {
        return entries();
    }

    inline void write(T value) {
        size_t write = _writePtr;
        _buffer[write] = value;
        _writePtr = (write + 1) % Size;
    }

    inline void write(const T *data, size_t length) {
        while (length--) {
            write(*data++);
        }
    }

    inline T read() {
        size_t read = _readPtr;
        T value = _buffer[read];
        _readPtr = (read + 1) % Size;
        return value;
    }

    inline void read(T &data) {
        size_t read = _readPtr;
        //memcpy(&data, &_buffer[read], sizeof(T));
        data = _buffer[read];
        _readPtr = (read + 1) % Size;
    }

    inline void read(T *data, size_t length) {
        while (length--) {
            read(*data++);
        }
    }

private:
    T _buffer[Size];
    std::atomic<int32_t> _readPtr;
    std::atomic<int32_t> _writePtr;
};
