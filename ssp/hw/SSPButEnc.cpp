
#include <ssp/hw/SSPButEnc.h>

#if defined(TARGET_SSP) && !defined(__APPLE__)

#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

#include <cstdio>

namespace ssp
{
    static constexpr int kEncoderMultiplier = 1;

    SSPButEnc::~SSPButEnc()
    {
        for (int i = 0; i < (int)NUM_ENCODERS; i++)
        {
            if (encFD_[i] >= 0)
            {
                close(encFD_[i]);
            }
        }

        if (encSwitchFD_ >= 0)
        {
            close(encSwitchFD_);
        }

        if (buttonFD_ >= 0)
        {
            close(buttonFD_);
        }
    }

    bool SSPButEnc::init()
    {
        for (int i = 0; i < (int)NUM_ENCODERS; i++)
        {
            static constexpr unsigned MAX_PATH = 100;
            char path[MAX_PATH];
            std::snprintf(path, MAX_PATH, "/dev/input/by-path/platform-rotary@%d-event", i);
            encFD_[i] = open(path, O_RDONLY | O_NONBLOCK);
            if (encFD_[i] < 0)
            {
                return false;
            }
        }

        encSwitchFD_ = open("/dev/input/by-path/platform-gpio-keys-event", O_RDONLY | O_NONBLOCK);
        if (encSwitchFD_ < 0)
        {
            return false;
        }

        buttonFD_ = open("/dev/input/by-path/platform-matrix-keypad-event", O_RDONLY | O_NONBLOCK);
        if (buttonFD_ < 0)
        {
            return false;
        }

        initialized_ = true;
        return true;
    }

    bool SSPButEnc::isAvailable() const
    {
        return initialized_;
    }

    SSPButtonId SSPButEnc::mapButtonCode(int code) const
    {
        switch (code)
        {
        case 88:
            return SSPButtonId::Soft1;
        case 87:
            return SSPButtonId::Soft2;
        case 68:
            return SSPButtonId::Soft3;
        case 67:
            return SSPButtonId::Soft4;
        case 64:
            return SSPButtonId::Soft5;
        case 63:
            return SSPButtonId::Soft6;
        case 62:
            return SSPButtonId::Soft7;
        case 61:
            return SSPButtonId::Soft8;
        case 65:
            return SSPButtonId::Up;
        case 59:
            return SSPButtonId::Down;
        case 66:
            return SSPButtonId::LShift;
        case 187:
            return SSPButtonId::RShift;
        case 60:
            return SSPButtonId::Left;
        case 188:
            return SSPButtonId::Right;
        case 183:
            return SSPButtonId::P1;
        case 184:
            return SSPButtonId::P2;
        case 185:
            return SSPButtonId::P3;
        case 186:
            return SSPButtonId::P4;
        default:
            return SSPButtonId::Invalid;
        }
    }

    void SSPButEnc::poll(
        const std::function<void(SSPButtonId, bool)> &onButton,
        const std::function<void(SSPEncoderId, int)> &onEncoderTurn,
        const std::function<void(SSPEncoderId, bool)> &onEncoderPress)
    {
        if (!initialized_)
        {
            return;
        }

        bool moreEvents = true;
        while (moreEvents)
        {
            input_event ev;
            moreEvents = false;

            for (int i = 0; i < (int)NUM_ENCODERS; i++)
            {
                if (read(encFD_[i], &ev, sizeof(ev)) == sizeof(ev))
                {
                    if (ev.type == EV_REL && ev.code == REL_X)
                    {
                        int mapped = encMap_[i];
                        int delta = ev.value * kEncoderMultiplier;
                        if (delta != 0 && mapped >= 0 && mapped < (int)NUM_ENCODERS)
                        {
                            onEncoderTurn((SSPEncoderId)mapped, delta);
                        }
                    }
                    moreEvents = true;
                }
            }

            if (read(encSwitchFD_, &ev, sizeof(ev)) == sizeof(ev))
            {
                if (ev.type == EV_KEY)
                {
                    int logicalIndex = ev.code - 2;
                    if (logicalIndex >= 0 && logicalIndex < (int)NUM_ENCODERS)
                    {
                        int mapped = encSwMap_[logicalIndex];
                        if (mapped >= 0 && mapped < (int)NUM_ENCODERS)
                        {
                            onEncoderPress((SSPEncoderId)mapped, ev.value != 0);
                        }
                    }
                }
                moreEvents = true;
            }

            if (read(buttonFD_, &ev, sizeof(ev)) == sizeof(ev))
            {
                if (ev.type == EV_KEY)
                {
                    SSPButtonId mapped = mapButtonCode(ev.code);
                    if (mapped != SSPButtonId::Invalid)
                    {
                        onButton(mapped, ev.value != 0);
                    }
                }
                moreEvents = true;
            }
        }
    }
} // namespace ssp

#else

namespace ssp
{
    SSPButEnc::~SSPButEnc()
    {
    }

    bool SSPButEnc::init()
    {
        initialized_ = false;
        return false;
    }

    bool SSPButEnc::isAvailable() const
    {
        return false;
    }

    SSPButtonId SSPButEnc::mapButtonCode(int code) const
    {
        (void)code;
        return SSPButtonId::Invalid;
    }

    void SSPButEnc::poll(
        const std::function<void(SSPButtonId, bool)> &onButton,
        const std::function<void(SSPEncoderId, int)> &onEncoderTurn,
        const std::function<void(SSPEncoderId, bool)> &onEncoderPress)
    {
        (void)onButton;
        (void)onEncoderTurn;
        (void)onEncoderPress;
    }
} // namespace ssp

#endif
