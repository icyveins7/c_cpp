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

struct SettingsRxUSRP
{
    std::string make_args = ""; // 1
    std::string subdev_args = ""; // 2
    std::vector<std::string> channel_strings = {"0"}; // 3
    std::string clock_source = "internal"; // 4
    std::string time_source = "internal"; // 4
    double global_rate = 0;
    std::vector<double> channel_rates;

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
    std::vector<size_t> rx_channel_nums; // TODO: redo this to just incorporate the channel settings directly ie freq, gain, filter bw, antenna
    for (size_t ch = 0; ch < settings.channel_strings.size(); ch++) {
        size_t chan = std::stoi(settings.channel_strings[ch]);
        if (chan >= rx_usrp->get_rx_num_channels()) {
            throw std::runtime_error("Invalid RX channel(s) specified.");
        } else
            rx_channel_nums.push_back(std::stoi(settings.channel_strings[ch]));
    }

    // 4. Lock clock and time sources
    rx_usrp->set_clock_source(settings.clock_source);
    rx_usrp->set_time_source(settings.time_source);

    // 5. 
};