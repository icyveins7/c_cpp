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
    size_t channel = uhd::usrp::multi_usrp::ALL_CHANS;
    double rate = 0;
    double freq = 0;
    double lo_offset = 0;
    double gain = 0;
    double bw = 0;
    std::string ant = "RX2";
};

struct SettingsUSRP
{
    std::string make_args = ""; // 1
    std::string subdev_args = ""; // 2

    std::string clock_source = "internal"; // 3
    std::string time_source = "internal"; // 3

    // All channel specific settings
    std::vector<ChannelSetting> chnlsettings; // 4
};

/*
A simple, standard function to initialise all common settings on a USRP receiver object.
Use in conjunction with the SettingsUSRP struct above.
*/
void setupRxUSRP(
    uhd::usrp::multi_usrp::sptr rx_usrp,
    SettingsUSRP &settings
    )
{
    // 1. Make
    rx_usrp = uhd::usrp::multi_usrp::make(settings.make_args);

    // 2. Set subdevice if not empty
    rx_usrp->set_rx_subdev_spec(settings.subdev_args);

    // 3. Lock clock and time sources
    rx_usrp->set_clock_source(settings.clock_source);
    rx_usrp->set_time_source(settings.time_source);

    // 4. Channel specific settings
    for (int i = 0; i < settings.chnlsettings.size(); i++)
    {
        // 4a. Rate
        rx_usrp->set_rx_rate(
            settings.chnlsettings.at(i).rate,
            settings.chnlsettings.at(i).channel);

        // 4b. Frequencies + LOs
        uhd::tune_request_t tune_request(
            settings.chnlsettings.at(i).freq,
            settings.chnlsettings.at(i).lo_offset
        );
        rx_usrp->set_rx_freq(
            tune_request,
            settings.chnlsettings.at(i).channel);

        // 4c. Gain
        rx_usrp->set_rx_gain(
            settings.chnlsettings.at(i).gain,
            settings.chnlsettings.at(i).channel
        );

        // 4d. Filter BW (conditional setting)
        if (settings.chnlsettings.at(i).bw != 0)
        {
            rx_usrp->set_rx_bandwidth(
                settings.chnlsettings.at(i).bw,
                settings.chnlsettings.at(i).channel
            );
        }

        // 4e. Antenna
        rx_usrp->set_rx_antenna(
            settings.chnlsettings.at(i).ant,
            settings.chnlsettings.at(i).channel
        );
    }
};


/*
A simple, standard function to initialise all common settings on a USRP transmitter object.
Use in conjunction with the SettingsUSRP struct above.
*/
void setupTxUSRP(
    uhd::usrp::multi_usrp::sptr tx_usrp,
    SettingsUSRP &settings
    )
{
    // 1. Make
    tx_usrp = uhd::usrp::multi_usrp::make(settings.make_args);

    // 2. Set subdevice if not empty
    tx_usrp->set_tx_subdev_spec(settings.subdev_args);

    // 3. Lock clock and time sources
    tx_usrp->set_clock_source(settings.clock_source);
    tx_usrp->set_time_source(settings.time_source);

    // 4. Channel specific settings
    for (int i = 0; i < settings.chnlsettings.size(); i++)
    {
        // 4a. Rate
        tx_usrp->set_tx_rate(
            settings.chnlsettings.at(i).rate,
            settings.chnlsettings.at(i).channel);

        // 4b. Frequencies + LOs
        uhd::tune_request_t tune_request(
            settings.chnlsettings.at(i).freq,
            settings.chnlsettings.at(i).lo_offset
        );
        tx_usrp->set_tx_freq(
            tune_request,
            settings.chnlsettings.at(i).channel);

        // 4c. Gain
        tx_usrp->set_tx_gain(
            settings.chnlsettings.at(i).gain,
            settings.chnlsettings.at(i).channel
        );

        // 4d. Filter BW (conditional setting)
        if (settings.chnlsettings.at(i).bw != 0)
        {
            tx_usrp->set_tx_bandwidth(
                settings.chnlsettings.at(i).bw,
                settings.chnlsettings.at(i).channel
            );
        }

        // 4e. Antenna
        tx_usrp->set_tx_antenna(
            settings.chnlsettings.at(i).ant,
            settings.chnlsettings.at(i).channel
        );
    }
};
