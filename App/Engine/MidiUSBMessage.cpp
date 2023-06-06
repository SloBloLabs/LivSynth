#include "MidiUSBMessage.h"

MidiUSBMessage::MidiUSBMessage(uint8_t *data) {
    memcpy(_raw, data, 4);
}

MidiUSBMessage::MidiUSBMessage(const uint8_t cableNumber, MidiMessage &midiMessage) :
        MidiUSBMessage(cableNumber, getChannelIndexNumber(midiMessage.channelMessage()), midiMessage) {}

MidiUSBMessage::MidiUSBMessage(const uint8_t cableNumber, const uint8_t codeIndexNumber, MidiMessage &midiMessage) :
        _raw{0, midiMessage.status(), midiMessage.data0(), midiMessage.data1()} {
    setCableNumber(cableNumber);
    setCodeIndexNumber(codeIndexNumber);
}

ChannelIndexNumber MidiUSBMessage::getChannelIndexNumber(MidiMessage::ChannelMessage status) {
    ChannelIndexNumber ret;
    switch(status) {
    case MidiMessage::ChannelMessage::NoteOff:
        ret = NOTE_OFF;
        break;
    case MidiMessage::ChannelMessage::NoteOn:
        ret = NOTE_ON;
        break;
    case MidiMessage::ChannelMessage::KeyPressure:
        ret = POLYPHONIC_KEY_PRESSURE;
        break;
    case MidiMessage::ChannelMessage::ControlChange:
        ret = CONTROL_CHANGE;
        break;
    case MidiMessage::ChannelMessage::ProgramChange:
        ret = PROGRAM_CHANGE;
        break;
    case MidiMessage::ChannelMessage::ChannelPressure:
        ret = CHANNEL_PRESSURE;
        break;
    case MidiMessage::ChannelMessage::PitchBend:
        ret = PITCH_BEND;
        break;
    default:
        ret = INVALID;
        break;
    }
    return ret;
}

void MidiUSBMessage::dump(const MidiUSBMessage &msg) {
    printf("DumpUSBMessage: data0: %02x, data1: %02x, data2: %02x, data3: %02x \n", msg._raw[0], msg._raw[1], msg._raw[2], msg._raw[3]);
}
