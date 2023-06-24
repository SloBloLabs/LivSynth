#pragma once

#include "RingBuffer.h"
#include "MidiMessage.h"
#include "MidiUSBMessage.h"
#include "usbd_midi_if.h"

#include <functional>

#define MAX_ATTEMPTS      10

extern USBD_MIDI_ItfTypeDef USBD_MIDI_fops_FS;

class MidiHandler {
public:
    typedef std::function<bool(MidiMessage&)> RealtimeHandler;

    MidiHandler() = default;
    ~MidiHandler() = default;

    inline void init() {
        _incoming.init();
        _outgoing.init();
        _cableNumber = 0;
        _busy = false;
        _rtHandler = nullptr;
        memset(_sendBuffer, 0, USB_FS_MAX_PACKET_SIZE);
    }

    inline void enqueueOutgoing(MidiMessage &msg) {
        _outgoing.write(msg);
    }

    inline bool outgoingIsFull() {
        return _outgoing.full();
    }

    inline bool outgoingIsEmpty() {
        return _outgoing.empty();
    }

    inline bool dequeueOutgoing(MidiMessage* msg) {
        if(outgoingIsEmpty()) {
            return false;
        }
        _outgoing.read(msg, 1);
        return true;
    }

    // called by USB ISR
    inline void enqueueIncoming(MidiMessage &msg) {
        if(_rtHandler && msg.isRealTimeMessage()) {
            _rtHandler(msg);
        }
        _incoming.write(msg);
    }

    inline bool incomingIsFull() {
        return _incoming.full();
    }

    inline bool incomingIsEmpty() {
        return _incoming.empty();
    }

    inline bool dequeueIncoming(MidiMessage* msg) {
        if(incomingIsEmpty()) {
            return false;
        }
        _incoming.read(msg, 1);
        return true;
    }

    inline void processOutgoing() {
        if(busy()) {
            return;
        }
        
        setBusy(true);

        MidiMessage msg;
        uint8_t n = 0;
        uint8_t ret; // USBD_StatusTypeDef

        // collect messages into send buffer
        while( (n < USB_FS_MAX_PACKET_SIZE) && dequeueOutgoing(&msg)) {
            MidiUSBMessage umsg(_cableNumber, msg);
            //MidiUSBMessage::dump(umsg);

            memcpy((_sendBuffer + n), umsg.getData(), 4);
            n += 4;
        }

        //printf("%d message bytes collected\n", n);

        // send messages to USB
        uint8_t i = 0;
        do {
            ret = USBD_MIDI_fops_FS.Send(_sendBuffer, n);

            //if(i > 0)
            //  printf("USB Send #%d: status = %d\n", i, ret);
        } while(ret != USBD_OK && i++ < MAX_ATTEMPTS);
    }

    inline uint8_t cableNumber() const {
        return _cableNumber;
    }

    inline void setCableNumber(const uint8_t cableNumber) {
        _cableNumber = cableNumber;
    }

    inline bool busy() const {
        return _busy;
    }

    inline void setBusy(const bool busy) {
        _busy = busy;
    }

    void setRealtimeHandler(RealtimeHandler rtHandler) {
        _rtHandler = rtHandler;
    }

private:
    RingBuffer<MidiMessage, USB_FS_MAX_PACKET_SIZE << 1> _incoming;
    RingBuffer<MidiMessage, USB_FS_MAX_PACKET_SIZE << 1> _outgoing;

    uint8_t _cableNumber;
    volatile bool _busy;

    uint8_t _sendBuffer[USB_FS_MAX_PACKET_SIZE];

    RealtimeHandler _rtHandler;

};
