#pragma once

struct ILEDController
{
#pragma pack(push, 1)
    struct LEDState {
        bool     on;
        uint16_t warm;
        uint16_t cool;
    };
#pragma pack(pop)

    virtual LEDState GetState() = 0;
    virtual void SetState(const LEDState& state) = 0;
};
