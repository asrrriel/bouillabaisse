#include "spdlog/spdlog.h"
#include <alsa/asoundlib.h>
#include <io/Alsa.hpp>

int InputAudioDevice::open_stream(snd_pcm_t **handle, unsigned int sampleRate,
                                  unsigned int channels,
                                  snd_pcm_format_t format) {
    int err;
    std::string dev_str = get_dev_string();

    if ((err = snd_pcm_open(handle, dev_str.c_str(), SND_PCM_STREAM_CAPTURE,
                            0)) < 0) {
        spdlog::error("Failed to open PCM device {}: {}", dev_str,
                      snd_strerror(err));
        return err;
    }

    if ((err =
             snd_pcm_set_params(*handle, format, SND_PCM_ACCESS_RW_INTERLEAVED,
                                channels, sampleRate, 1, 500000)) < 0) {
        spdlog::error("Failed to set PCM parameters for device {}: {}", dev_str,
                      snd_strerror(err));
        snd_pcm_close(*handle);
        return err;
    }
    spdlog::info(
        "PCM device {} opened successfully with sample rate {} and channels {}",
        dev_str, sampleRate, channels);
    return 0;
}

std::vector<InputAudioDevice> AudioDeviceManager::get_input_devices() {
    std::vector<InputAudioDevice> input_devices;

    int card_number = -1;
    int err;

    if (snd_card_next(&card_number) < 0) {
        spdlog::warn("No sound cards found!");
        return input_devices;
    }

    while (card_number >= 0) {
        snd_ctl_t *ctl;
        snd_ctl_card_info_t *info;
        snd_ctl_card_info_alloca(&info);

        char card_id[32];
        snprintf(card_id, sizeof(card_id), "hw:%d", card_number);
        spdlog::info("Trying to open card {}", card_id);

        if ((err = snd_ctl_open(&ctl, card_id, 0)) < 0) {
            spdlog::warn("Failed to open card {}: {}", card_id,
                         snd_strerror(err));
            if (snd_card_next(&card_number) < 0)
                break;
        }

        if ((err = snd_ctl_card_info(ctl, info)) < 0) {
            spdlog::warn("Failed to get card info for {}: {}", card_id,
                         snd_strerror(err));
            if (snd_card_next(&card_number) < 0)
                break;
        }

        std::string card_name_str = snd_ctl_card_info_get_name(info);

        int dev_num = -1;
        while (true) {
            if (snd_ctl_pcm_next_device(ctl, &dev_num) < 0) {
                spdlog::warn("Failed to get next device for card {}: {}", card_id,
                             snd_strerror(err));
                break;
            }

            if (dev_num < 0)
                break;

            snd_pcm_info_t *pcminfo;
            snd_pcm_info_alloca(&pcminfo);
            snd_pcm_info_set_device(pcminfo, dev_num);
            snd_pcm_info_set_subdevice(pcminfo, 0);
            snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_CAPTURE);

            if ((err = snd_ctl_pcm_info(ctl, pcminfo)) < 0) {
                continue;
            }

            std::string device_name_str = snd_pcm_info_get_name(pcminfo);

            /*
             card_number(card_num), device_number(dev_num), card_name(card_n),
                      device_name(device_n), shared_card(shared_card),
                      shared_dev(shared_dev), flags(flags),
                      playback_subdevs_count(playback_subdevs_count),
                      capture_subdev_count(capture_subdev_count)
             * */

            input_devices.emplace_back(card_number, dev_num, card_name_str,
                                       device_name_str);
        }

        if (snd_card_next(&card_number) < 0)
            break;

        snd_ctl_close(ctl);
    }
    return input_devices;
}
