#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

#include <alsa/asoundlib.h>

#include <spdlog/spdlog.h>

class auDevice {
protected:
    int card_number;
    int device_number;

    int shared_card;
    int shared_dev;

    int flags;

    int playback_subdevs_count;
    int capture_subdev_count;

    uint8_t id[64];

    std::string card_name;
    std::string device_name;

public:
    auDevice (int card_num, int dev_num, std::string card_n,
              std::string device_n, int shared_card, int shared_dev, int flags,
              int playback_subdevs_count, int capture_subdev_count,
              uint8_t id[64]) :
        card_number (card_num), device_number (dev_num), card_name (card_n),
        device_name (device_n), shared_card (shared_card),
        shared_dev (shared_dev), flags (flags),
        playback_subdevs_count (playback_subdevs_count),
        capture_subdev_count (capture_subdev_count) {
        memcpy (this->id, id, 64);
    }

    auDevice (int cardNum, int devNum, std::string cardN, std::string devN) :
        card_number (cardNum), device_number (devNum),
        card_name (std::move (cardN)), device_name (std::move (devN)) {}
    virtual ~auDevice () = default;

    inline int
    get_card_number () const {
        return card_number;
    }

    inline int
    get_dev_number () const {
        return device_number;
    }

    inline std::string
    get_dev_name () const {
        return device_name;
    }

    inline std::string
    get_card_name () const {
        return card_name;
    }

    inline int
    get_shared_card () const {
        return shared_card;
    }

    inline int
    get_shared_dev () const {
        return shared_dev;
    }

    inline int
    get_flags () const {
        return flags;
    }

    inline int
    get_playback_subdevs_count () const {
        return playback_subdevs_count;
    }

    inline int
    get_capture_subdevs_count () const {
        return capture_subdev_count;
    }

    std::string
    get_dev_string () const {
        char buf[32];
        snprintf (buf, sizeof (buf), "pulse");
        return buf;
    }
};

class auInputDevice : public auDevice {

public:
    using auDevice::auDevice;

    int open_stream (snd_pcm_t **handle, unsigned int sampleRate = 48000,
                     unsigned int channels   = 2,
                     snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE);
};

class auOutputDevice : public auDevice {
public:
    using auDevice::auDevice;

    snd_pcm_t *handle = nullptr;

    unsigned int sample_rate = 48000;

    int open_stream (snd_pcm_t **handle, unsigned int sampleRate = 48000,
                     unsigned int channels   = 2,
                     snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE);

    int sin_play (uint32_t frequency = 440, uint32_t duration_ms = 1000);

    int write_frame (const void *data, size_t size);
};

class auDeviceManager {
public:
    std::vector<auInputDevice>  get_input_devices ();
    std::vector<auOutputDevice> get_output_devices ();
};
