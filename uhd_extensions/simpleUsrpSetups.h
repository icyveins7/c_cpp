#include <uhd/exception.hpp>
#include <uhd/types/tune_request.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/static.hpp>
#include <uhd/utils/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/thread.hpp>
#include <csignal>
#include <fstream>
#include <iostream>
#include <string>

struct ChannelSetting
{
    size_t channel = 0;
    double rate = 0;
    double freq = 0;
    double lo_offset = 0;
    double gain = 0;
    double bw = 0;
    std::string ant = "RX2";
};

struct SettingsRxUSRP
{
    std::string make_args = ""; // 1
    std::string subdev_args = ""; // 2
    std::vector<std::string> channel_strings = {"0"}; // 3
    std::string clock_source = "internal"; // 4
    std::string time_source = "internal"; // 4
    double global_rate = 0; // 5
    std::vector<double> channel_rates; // 5
    std::vector<double> get_rates; // 5 (returned)
    double global_freq = 0; // 6
    std::vector<double> channel_freqs; // 6
    std::vector<double> get_freqs; // 6

    // All channel specific settings
    std::vector<ChannelSetting> chnlsettings;
};

/*
A simple, standard function to initialise all common settings on a USRP receiver object.
Use in conjunction with the settings struct above.
*/
void setupRxUSRP(
    uhd::usrp::multi_usrp::sptr rx_usrp,
    SettingsRxUSRP settings
    )
{
    // 1. Make
    rx_usrp = uhd::usrp::multi_usrp::make(settings.make_args);

    // 2. Set subdevice if not empty
    rx_usrp->set_rx_subdev_spec(settings.subdev_args);

    // 3. Set channels to be used
    std::vector<size_t> channel_nums; // TODO: redo this to just incorporate the channel settings directly ie freq, gain, filter bw, antenna
    for (size_t ch = 0; ch < settings.channel_strings.size(); ch++) {
        size_t chan = std::stoi(settings.channel_strings[ch]);
        if (chan >= rx_usrp->get_rx_num_channels()) {
            throw std::runtime_error("Invalid RX channel(s) specified.");
        } else
            channel_nums.push_back(chan);
    }

    // 4. Lock clock and time sources
    rx_usrp->set_clock_source(settings.clock_source);
    rx_usrp->set_time_source(settings.time_source);

    // 5. Set sample rates
    if (settings.global_rate != 0)
    {
        // Then set one for everything, this is the most common scenario
        rx_usrp->set_rx_rate(settings.global_rate);
        settings.get_rates.push_back(rx_usrp->get_rx_rate());
    }
    else
    {
        // Otherwise we set one for each channel
        if (settings.channel_rates.size() != settings.channel_strings.size())
        {
            throw std::runtime_error(
                "Invalid number of RX sample rates specified, expected " + 
                std::to_string(settings.channel_strings.size()));
        }
        else
        {
            // Set all first
            for (int i = 0; i < settings.channel_strings.size(); i++)
            {
                rx_usrp->set_rx_rate(
                    settings.channel_rates.at(i),
                    channel_nums.at(i)
                );
            }
            // Then call the getters, in case later settings change the values of earlier ones
            for (int i = 0; i < settings.channel_strings.size(); i++)
            {
                settings.get_rates.push_back(
                    rx_usrp->get_rx_rate(channel_nums.at(i))
                );
            }
        }
    }

    // 6. Set centre frequencies, TODO: INCLUDE LO_OFFSETS
    if (settings.global_freq != 0)
    {
        uhd::tune_request_t tune_request(settings.global_freq);
        rx_usrp->set_rx_freq(tune_request);
        settings.get_freqs.push_back(rx_usrp->get_rx_freq());
    }
    else
    {
        // Otherwise we set one for each channel
        if (settings.channel_rates.size() != settings.channel_strings.size())
        {
            throw std::runtime_error(
                "Invalid number of RX centre frequencies specified, expected " + 
                std::to_string(settings.channel_strings.size()));
        }
        else
        {
            // Set all first
            for (int i = 0; i < settings.channel_strings.size(); i++)
            {
                uhd::tune_request_t tune_request(settings.channel_freqs.at(i));
                rx_usrp->set_rx_freq(
                    settings.channel_freqs.at(i),
                    channel_nums.at(i)
                );
            }
            // Then call the getters, in case later settings change the values of earlier ones
            for (int i = 0; i < settings.channel_strings.size(); i++)
            {
                settings.get_freqs.push_back(
                    rx_usrp->get_rx_freq(channel_nums.at(i))
                );
            }
        }
    }
};