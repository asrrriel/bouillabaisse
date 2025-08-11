#pragma once

#include "Audio.hpp"
#include <string>
#include <vector>

#include <alsa/asoundlib.h>

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
    std::string dev_string;

public:
    auDevice (int card_num, int dev_num, std::string card_n,
              std::string device_n, std::string dev_str, int shared_card,
              int shared_dev, int flags, int playback_subdevs_count,
              int capture_subdev_count, uint8_t id[64]) :
        card_number (card_num), device_number (dev_num),
        shared_card (shared_card), shared_dev (shared_dev), flags (flags),
        playback_subdevs_count (playback_subdevs_count),
        capture_subdev_count (capture_subdev_count), card_name (card_n),
        device_name (device_n), dev_string (dev_str) {
        memcpy (this->id, id, 64);
    }

    auDevice (int card_num, int dev_num, std::string card_name,
              std::string dev_name, std::string dev_str) :
        card_number (card_num), device_number (dev_num),
        card_name (std::move (card_name)), device_name (std::move (dev_name)),
        dev_string (std::move (dev_str)) {}
    virtual ~auDevice () = default;

    inline int get_card_number () const { return card_number; }

    inline int get_dev_number () const { return device_number; }

    inline std::string get_dev_name () const { return device_name; }

    inline std::string get_card_name () const { return card_name; }

    inline int get_shared_card () const { return shared_card; }

    inline int get_shared_dev () const { return shared_dev; }

    inline int get_flags () const { return flags; }

    inline int get_playback_subdevs_count () const {
        return playback_subdevs_count;
    }

    inline int get_capture_subdevs_count () const {
        return capture_subdev_count;
    }

    inline std::string get_dev_string () const { return dev_string; }
};

class auInputDevice : public auDevice {

public:
    using auDevice::auDevice;

    int open_stream (snd_pcm_t **handle, auSFormat s_format,
                     size_t latency = 500000);
};

class auOutputDevice : public auDevice {
public:
    using auDevice::auDevice;

    snd_pcm_t *handle = nullptr;

    unsigned int sample_rate = 48000;

    int open_stream (snd_pcm_t **handle, auSFormat s_format,
                     size_t latency = 500000);

    int play_chunk (const void *data, size_t num_frames);
};

class auDeviceManager {
public:
    std::vector<auInputDevice>  get_input_devices ();
    std::vector<auOutputDevice> get_output_devices ();
};
