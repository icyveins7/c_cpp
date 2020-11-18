#include <uhd.h>
#include <uhd/usrp/multi_usrp.hpp>
#include <string>
#include <iostream>
#include <stdint.h>
#include <chrono>

#include <fstream> // gonna remove this later

static bool stop_signal_called = false;
void sig_int_handler(int)
{
    stop_signal_called = true;
}

template <typename samp_type>
void recv_to_file(uhd::usrp::multi_usrp::sptr usrp,
    const std::string& cpu_format,
    const std::string& wire_format,
    const size_t& channel,
    const std::string& file,
    size_t samps_per_buff,
    unsigned long long num_requested_samples,
    double time_requested       = 0.0,
    bool bw_summary             = false,
    bool stats                  = false,
    bool null                   = false,
    bool enable_size_map        = false,
    bool continue_on_bad_packet = false)
{
    unsigned long long num_total_samps = 0;
    // create a receive streamer
    uhd::stream_args_t stream_args(cpu_format, wire_format);
    std::vector<size_t> channel_nums;
    channel_nums.push_back(channel);
    stream_args.channels             = channel_nums;
    uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);

    uhd::rx_metadata_t md;
    std::vector<samp_type> buff(samps_per_buff);
    std::ofstream outfile;
    if (not null)
        outfile.open(file.c_str(), std::ofstream::binary);
    bool overflow_message = true;

    // setup streaming
    uhd::stream_cmd_t stream_cmd((num_requested_samples == 0)
                                     ? uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS
                                     : uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);
    stream_cmd.num_samps  = size_t(num_requested_samples);
    stream_cmd.stream_now = true;
    stream_cmd.time_spec  = uhd::time_spec_t();
    rx_stream->issue_stream_cmd(stream_cmd);

    typedef std::map<size_t, size_t> SizeMap;
    SizeMap mapSizes;
    const auto start_time = std::chrono::steady_clock::now();
    const auto stop_time =
        start_time + std::chrono::milliseconds(int64_t(1000 * time_requested));
    // Track time and samps between updating the BW summary
    auto last_update                     = start_time;
    unsigned long long last_update_samps = 0;

	// read out the metadata each time?
	uhd::time_spec_t rxtime;
	
    // Run this loop until either time expired (if a duration was given), until
    // the requested number of samples were collected (if such a number was
    // given), or until Ctrl-C was pressed.
    while (not stop_signal_called
           and (num_requested_samples != num_total_samps or num_requested_samples == 0)
           and (time_requested == 0.0 or std::chrono::steady_clock::now() <= stop_time)) {
        const auto now = std::chrono::steady_clock::now();

        size_t num_rx_samps =
            rx_stream->recv(&buff.front(), buff.size(), md, 3.0, enable_size_map);

        if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_TIMEOUT) {
            std::cout << boost::format("Timeout while streaming") << std::endl;
            break;
        }
        if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_OVERFLOW) {
            if (overflow_message) {
                overflow_message = false;
                std::cerr
                    << boost::format(
                           "Got an overflow indication. Please consider the following:\n"
                           "  Your write medium must sustain a rate of %fMB/s.\n"
                           "  Dropped samples will not be written to the file.\n"
                           "  Please modify this example for your purposes.\n"
                           "  This message will not appear again.\n")
                           % (usrp->get_rx_rate(channel) * sizeof(samp_type) / 1e6);
            }
            continue;
        }
        if (md.error_code != uhd::rx_metadata_t::ERROR_CODE_NONE) {
            std::string error = str(boost::format("Receiver error: %s") % md.strerror());
            if (continue_on_bad_packet) {
                std::cerr << error << std::endl;
                continue;
            } else
                throw std::runtime_error(error);
        }
		
		// read the time spec
		rxtime = md.time_spec;
		printf("Full secs : %lld, frac secs : %.11f\n", rxtime.get_full_secs(), rxtime.get_frac_secs());
		
		// attempt to convert tick rate?
		// here we define a constant tick rate to calculate the ticks
		printf("Ticks: %lld\n", rxtime.to_ticks(100000000.0));
		// seems like this can basically be used to track sample number (kinda)

        if (enable_size_map) {
            SizeMap::iterator it = mapSizes.find(num_rx_samps);
            if (it == mapSizes.end())
                mapSizes[num_rx_samps] = 0;
            mapSizes[num_rx_samps] += 1;
        }

        num_total_samps += num_rx_samps;

		// // ignore the writes for now..
        // if (outfile.is_open()) {
            // outfile.write((const char*)&buff.front(), num_rx_samps * sizeof(samp_type));
        // }

        if (bw_summary) {
            last_update_samps += num_rx_samps;
            const auto time_since_last_update = now - last_update;
            if (time_since_last_update > std::chrono::seconds(1)) {
                const double time_since_last_update_s =
                    std::chrono::duration<double>(time_since_last_update).count();
                const double rate = double(last_update_samps) / time_since_last_update_s;
                std::cout << "\t" << (rate / 1e6) << " Msps" << std::endl;
                last_update_samps = 0;
                last_update       = now;
            }
        }
    }
    const auto actual_stop_time = std::chrono::steady_clock::now();

    stream_cmd.stream_mode = uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS;
    rx_stream->issue_stream_cmd(stream_cmd);

    if (outfile.is_open()) {
        outfile.close();
    }

    if (stats) {
        std::cout << std::endl;
        const double actual_duration_seconds =
            std::chrono::duration<float>(actual_stop_time - start_time).count();

        std::cout << boost::format("Received %d samples in %f seconds") % num_total_samps
                         % actual_duration_seconds
                  << std::endl;
        const double rate = (double)num_total_samps / actual_duration_seconds;
        std::cout << (rate / 1e6) << " Msps" << std::endl;

        if (enable_size_map) {
            std::cout << std::endl;
            std::cout << "Packet size map (bytes: count)" << std::endl;
            for (SizeMap::iterator it = mapSizes.begin(); it != mapSizes.end(); it++)
                std::cout << it->first << ":\t" << it->second << std::endl;
        }
    }
}

void setUSRP_RXsettings(uhd::usrp::multi_usrp::sptr usrp, double rate=1e6, double freq=1.5e9, double gain=0, double lo_offset=0, size_t channel=0,
						int64_t full_secs=0, double frac_secs=0, bool specifyTime=false)
{
	usrp->set_rx_rate(rate, channel);
    printf("Actual RX Rate: %f Msps...\n", usrp->get_rx_rate(channel) / 1e6);
	
	uhd::tune_request_t tune_request(freq, lo_offset);

	usrp->set_rx_freq(tune_request, channel);
	printf("Actual RX Freq: %f MHz...\n", usrp->get_rx_freq(channel) / 1e6);
	
	usrp->set_rx_gain(gain, channel);
	printf("Actual RX Gain: %f dB...\n",usrp->get_rx_gain(channel));		

	// set the times AFTER setting the rates (internal clocks change after changing rates)
	// current time source
	printf("Current time source : %s\n", usrp->get_time_source(0).c_str());
	
	// get time now if time is 0
	uhd::time_spec_t t;
	if (specifyTime){
		t = uhd::time_spec_t(full_secs,frac_secs);
	}
	else{
		auto now = std::chrono::system_clock::now().time_since_epoch();
		// auto nowsecs = std::chrono::duration_cast<std::chrono::seconds>(now).count();
		double nowsecs = std::chrono::duration<double>(now).count();
		printf("Seconds since epoch = %.6f \n", nowsecs);
		
		t = uhd::time_spec_t(nowsecs);
	}
	usrp->set_time_now(t);
	std::cout << "Time set now to " << usrp->get_time_now().get_real_secs() << std::endl;
	
}

int main()
{
	std::string args("");
	uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);

	// set some settings
	setUSRP_RXsettings(usrp);

	// do the recording
	std::string cpu_format("sc16");
	std::string wire_format("sc16");
	size_t channel = 0;
	std::string file("usrpSamples.dat");
	size_t samps_per_buff = 100000;
	unsigned long long num_requested_samples = 3000000;

	recv_to_file<std::complex<short>>(
			usrp,
			cpu_format,
			wire_format,
			channel,
			file,
			samps_per_buff,
			num_requested_samples);

	return 0;
}