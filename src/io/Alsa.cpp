#include "spdlog/spdlog.h"
#include <alsa/asoundlib.h>
#include <fmt/core.h>
#include <io/Alsa.hpp>

snd_pcm_format_t sformat_to_pcm_format (auSFormat s_format) {
    switch (s_format.data_type) {
    case auDtype::uInt:
        switch (s_format.bit_depth) {
        case 8:
            return SND_PCM_FORMAT_U8;
        case 16:
            return SND_PCM_FORMAT_U16;
        case 24:
            return SND_PCM_FORMAT_U24;
        case 32:
            return SND_PCM_FORMAT_U32;
        default:
            return SND_PCM_FORMAT_U16; // lmao alsa doesnt even support 64 bit
                                       // audio
        }
    case auDtype::sInt:
        switch (s_format.bit_depth) {
        case 8:
            return SND_PCM_FORMAT_S8;
        case 16:
            return SND_PCM_FORMAT_S16;
        case 24:
            return SND_PCM_FORMAT_S24;
        case 32:
            return SND_PCM_FORMAT_S32;
        default:
            return SND_PCM_FORMAT_S16; // lmao alsa doesnt even support 64 bit
                                       // audio
        }
    case auDtype::sFloat:
        return SND_PCM_FORMAT_FLOAT;
    case auDtype::sDouble:
        return SND_PCM_FORMAT_FLOAT64;
    case auDtype::uALaw:
        return SND_PCM_FORMAT_A_LAW;
    case auDtype::uMuLaw:
        return SND_PCM_FORMAT_MU_LAW;
    case auDtype::uDviAdpcm:
        return SND_PCM_FORMAT_IMA_ADPCM;
    case auDtype::uMsAdpcm:
        return SND_PCM_FORMAT_IMA_ADPCM;
    default:
        return SND_PCM_FORMAT_S16_LE;
    }
}

int auInputDevice::open_stream (snd_pcm_t **handle, auSFormat s_format,
                                size_t latency) {
    int         err;
    std::string dev_str = get_dev_string ();

    if ((err
         = snd_pcm_open (handle, dev_str.c_str (), SND_PCM_STREAM_CAPTURE, 0))
        < 0) {
        spdlog::error ("Failed to open PCM device {}: {}", dev_str,
                       snd_strerror (err));
        return err;
    }

    if ((err = snd_pcm_set_params (*handle, sformat_to_pcm_format (s_format),
                                   SND_PCM_ACCESS_RW_INTERLEAVED,
                                   s_format.channels, s_format.sample_rate, 1,
                                   latency))
        < 0) {
        spdlog::error ("Failed to set PCM parameters for device {}: {}",
                       dev_str, snd_strerror (err));
        snd_pcm_close (*handle);
        return err;
    }
    spdlog::info ("PCM device {} opened successfully with sample rate {} and "
                  "channels {}",
                  dev_str, s_format.sample_rate, s_format.channels);
    return 0;
}

int auOutputDevice::open_stream (snd_pcm_t **handle, auSFormat s_format,
                                 size_t latency) {
    int         err;
    std::string dev_str = get_dev_string ();

    if ((err
         = snd_pcm_open (handle, dev_str.c_str (), SND_PCM_STREAM_PLAYBACK, 0))
        < 0) {
        spdlog::error ("Failed to open PCM device {}: {}", dev_str,
                       snd_strerror (err));
        return err;
    }

    snd_pcm_format_t fmt = sformat_to_pcm_format (s_format);

    spdlog::info ("Trying to open PCM device {} with format {}", dev_str,
                  snd_pcm_format_name (fmt));

    if ((err = snd_pcm_set_params (*handle, fmt, SND_PCM_ACCESS_RW_INTERLEAVED,
                                   s_format.channels, s_format.sample_rate, 1,
                                   latency))
        < 0) {
        spdlog::error ("Failed to set PCM parameters for device {}: {}",
                       dev_str, snd_strerror (err));
        snd_pcm_close (*handle);
        return err;
    }
    spdlog::info ("PCM device {} opened successfully with sample rate {} and "
                  "channels {}",
                  dev_str, s_format.sample_rate, s_format.channels);

    this->handle      = *handle;
    this->sample_rate = s_format.sample_rate;
    return 0;
}

int auOutputDevice::play_chunk (const void *data, size_t num_frames) {
    if (!handle) {
        spdlog::error ("Output device wasnt opened yet!");
        return -1;
    }

    snd_pcm_sframes_t written = snd_pcm_writei (handle, data, num_frames);
    if (written < 0) {
        spdlog::error ("Failed to write to PCM device: {}",
                       snd_strerror (written));
        return written;
    } else if (written < static_cast<snd_pcm_sframes_t> (num_frames)) {
        spdlog::warn ("Short write: expected {}, wrote {}", num_frames,
                      written);
    }

    return 0;
}

std::vector<auOutputDevice> auDeviceManager::get_output_devices () {
    std::vector<auOutputDevice> output_devices;

    output_devices.emplace_back (0, 0, "pulse", "pulse", "pulse");

    int card_number = -1;
    int err;

    if (snd_card_next (&card_number) < 0) {
        spdlog::warn ("No sound cards found!");
        return output_devices;
    }

    while (card_number >= 0) {
        snd_ctl_t           *ctl;
        snd_ctl_card_info_t *info;
        snd_ctl_card_info_alloca (&info);

        char card_id[32];
        snprintf (card_id, sizeof (card_id), "hw:%d", card_number);
        spdlog::info ("Trying to open card {}", card_id);

        if ((err = snd_ctl_open (&ctl, card_id, 0)) < 0) {
            spdlog::warn ("Failed to open card {}: {}", card_id,
                          snd_strerror (err));
            if (snd_card_next (&card_number) < 0) break;
        }

        if ((err = snd_ctl_card_info (ctl, info)) < 0) {
            spdlog::warn ("Failed to get card info for {}: {}", card_id,
                          snd_strerror (err));
            if (snd_card_next (&card_number) < 0) break;
        }

        std::string card_name_str = snd_ctl_card_info_get_name (info);

        int dev_num = -1;
        while (true) {
            if (snd_ctl_pcm_next_device (ctl, &dev_num) < 0) {
                spdlog::warn ("Failed to get next device for card {}: {}",
                              card_id, snd_strerror (err));
                break;
            }

            if (dev_num < 0) break;

            snd_pcm_info_t *pcminfo;
            snd_pcm_info_alloca (&pcminfo);
            snd_pcm_info_set_device (pcminfo, dev_num);
            snd_pcm_info_set_subdevice (pcminfo, 0);
            snd_pcm_info_set_stream (pcminfo, SND_PCM_STREAM_PLAYBACK);

            if ((err = snd_ctl_pcm_info (ctl, pcminfo)) < 0) { continue; }

            std::string device_name_str = snd_pcm_info_get_name (pcminfo);

            std::string device_string
                = fmt::format ("hw:{},{}", card_number, dev_num);

            output_devices.emplace_back (card_number, dev_num, card_name_str,
                                         device_name_str, device_string);
        }

        if (snd_card_next (&card_number) < 0) break;

        snd_ctl_close (ctl);
    }
    return output_devices;
}

std::vector<auInputDevice> auDeviceManager::get_input_devices () {
    std::vector<auInputDevice> input_devices;

    input_devices.emplace_back (0, 0, "pulse", "pulse", "pulse");

    int card_number = -1;
    int err;

    if (snd_card_next (&card_number) < 0) {
        spdlog::warn ("No sound cards found!");
        return input_devices;
    }

    while (card_number >= 0) {
        snd_ctl_t           *ctl;
        snd_ctl_card_info_t *info;
        snd_ctl_card_info_alloca (&info);

        char card_id[32];
        snprintf (card_id, sizeof (card_id), "hw:%d", card_number);
        spdlog::info ("Trying to open card {}", card_id);

        if ((err = snd_ctl_open (&ctl, card_id, 0)) < 0) {
            spdlog::warn ("Failed to open card {}: {}", card_id,
                          snd_strerror (err));
            if (snd_card_next (&card_number) < 0) break;
        }

        if ((err = snd_ctl_card_info (ctl, info)) < 0) {
            spdlog::warn ("Failed to get card info for {}: {}", card_id,
                          snd_strerror (err));
            if (snd_card_next (&card_number) < 0) break;
        }

        std::string card_name_str = snd_ctl_card_info_get_name (info);

        int dev_num = -1;
        while (true) {
            if (snd_ctl_pcm_next_device (ctl, &dev_num) < 0) {
                spdlog::warn ("Failed to get next device for card {}: {}",
                              card_id, snd_strerror (err));
                break;
            }

            if (dev_num < 0) break;

            snd_pcm_info_t *pcminfo;
            snd_pcm_info_alloca (&pcminfo);
            snd_pcm_info_set_device (pcminfo, dev_num);
            snd_pcm_info_set_subdevice (pcminfo, 0);
            snd_pcm_info_set_stream (pcminfo, SND_PCM_STREAM_CAPTURE);

            if ((err = snd_ctl_pcm_info (ctl, pcminfo)) < 0) { continue; }

            std::string device_name_str = snd_pcm_info_get_name (pcminfo);

            std::string device_string
                = fmt::format ("hw:{},{}", card_number, dev_num);

            input_devices.emplace_back (card_number, dev_num, card_name_str,
                                        device_name_str, device_string);
        }

        if (snd_card_next (&card_number) < 0) break;

        snd_ctl_close (ctl);
    }
    return input_devices;
}
