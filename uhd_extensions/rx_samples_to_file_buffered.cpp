//
// Copyright 2010-2011,2014 Ettus Research LLC
// Copyright 2018 Ettus Research, a National Instruments Company
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include <uhd/exception.hpp>
#include <uhd/types/tune_request.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/thread.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <chrono>
#include <complex>
#include <csignal>
#include <fstream>
#include <iostream>
#include <thread>

#ifdef linux
const char pathsplit = '/';
#else
const char pathsplit = '\\';
#endif

namespace po = boost::program_options;

static bool stop_signal_called = false;
void sig_int_handler(int)
{
    stop_signal_called = true;
}

template <typename samp_type>
void save_to_file(const std::string& folder, long long int second, std::vector<samp_type> &recdata, double threshold, double saturation_warning)
{
	char filename[512];
	snprintf(filename, 512, "%s%c%lld.bin", folder.c_str(), pathsplit, second);
    // printf("Writing to %s\n", filename);
	
    bool toWrite = false;
    if (threshold > 0)
    {
        // check if any values satisfy threshold
        toWrite = std::any_of(recdata.cbegin(), recdata.cend(), [threshold](samp_type val){return static_cast<double>(std::abs(val)) > threshold;});
    }
    else{
        toWrite = true; // if threshold is 0, always write (default behaviour)
    }

    // check for saturation if specified
    if (saturation_warning > 0 && std::any_of(recdata.cbegin(), recdata.cend(), [saturation_warning](samp_type val){return static_cast<double>(std::abs(val)) > saturation_warning;}))
        printf("Saturated samples found (> %.2f)", saturation_warning);

    
    if (toWrite)
    {
        FILE *fp = fopen(filename, "wb");
        if (fp != NULL)
        {
            fwrite(recdata.data(), sizeof(samp_type), recdata.size(), fp);
            fclose(fp);
            
            printf("Wrote %s.\n", filename);
        }
    }
	
}

template <typename samp_type>
void recv_to_file(uhd::usrp::multi_usrp::sptr usrp,
    const std::string& cpu_format,
    const std::string& wire_format,
    std::vector<size_t> &channel_nums,

    size_t samps_per_buff,
    unsigned long long num_requested_samples,
    std::vector<std::string> &folders,
    double threshold            = 0,
    double saturation_warning   = 0,
    double time_requested       = 0.0,
    bool bw_summary             = false,
    bool stats                  = false,
    bool null                   = false,
    bool enable_size_map        = false,
    bool verbose                = false)
{
    unsigned long long num_total_samps = 0;
    // create a receive streamer
    uhd::stream_args_t stream_args(cpu_format, wire_format);
    std::cout << "cpu_format: " << cpu_format << " wire_format: " << wire_format << std::endl;
    // std::vector<size_t> channel_nums;
    // channel_nums.push_back(channel);
    stream_args.channels             = channel_nums;
    uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);
	
	// adjust timing if using internal reference clocks (we have moved this to after streamer creation, since streamer creation screws it up?)
	if (usrp->get_time_source(0) == "internal"){
		std::cout << "========= Using internal clock. Adjusting USRP time to computer time." << std::endl;
		auto now = std::chrono::system_clock::now().time_since_epoch();
		// auto nowsecs = std::chrono::duration_cast<std::chrono::seconds>(now).count();
		double nowsecs = std::chrono::duration<double>(now).count();
		printf("========= Seconds since epoch = %.6f \n", nowsecs);
		
		uhd::time_spec_t t(nowsecs);
		usrp->set_time_now(t);
		printf("========= Time set now to %.6f\n", usrp->get_time_now().get_real_secs());
	}

    else if (usrp->get_time_source(0) == "gpsdo"){
        std::cout << "Using time source " << usrp->get_time_source(0) << std::endl;
        std::cout << "Using clock source " << usrp->get_clock_source(0) << std::endl;


        // Set to GPS time
        uhd::time_spec_t gps_time = uhd::time_spec_t(
            int64_t(usrp->get_mboard_sensor("gps_time", 0).to_int()));
        usrp->set_time_next_pps(gps_time + 1.0, 0);

        // Wait for it to apply
        // The wait is 2 seconds because N-Series has a known issue where
        // the time at the last PPS does not properly update at the PPS edge
        // when the time is actually set.
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Check times
        gps_time = uhd::time_spec_t(
            int64_t(usrp->get_mboard_sensor("gps_time", 0).to_int()));
        uhd::time_spec_t time_last_pps = usrp->get_time_last_pps(0);
        std::cout << "USRP time: "
            << (boost::format("%0.9f") % time_last_pps.get_real_secs())
            << std::endl;
        std::cout << "GPSDO time: "
            << (boost::format("%0.9f") % gps_time.get_real_secs()) << std::endl;
        if (gps_time.get_real_secs() == time_last_pps.get_real_secs()) {
            std::cout << std::endl
                << "SUCCESS: USRP time synchronized to GPS time" << std::endl
                << std::endl;
        }
        else {
            std::cerr << std::endl
                << "ERROR: Failed to synchronize USRP time to GPS time"
                << std::endl
                << std::endl;
        }
    }

    else {
        std::cout << "Not configured for non-internal/gpsdo sources yet!" << std::endl;
    }

    uhd::rx_metadata_t md;
	int rx_rate = static_cast<int>(round(usrp->get_rx_rate(channel_nums[0])));
	std::cout << "======= Using 1 second buffers (x2) of " << rx_rate << " length." << std::endl;
	// First make the actual buffers for each channel, x2 
    std::vector<std::vector<samp_type>> buffs[2]; // at the start, buffs[0] will be used, then buffs[1] after 1 second, and alternating thenceforth
	buffs[0].resize(channel_nums.size());
	buffs[1].resize(channel_nums.size());
	for (int ch = 0; ch < channel_nums.size(); ch++){
		// Reserve+resize buff1
		buffs[0].at(ch).reserve(rx_rate);
		buffs[0].at(ch).resize(rx_rate);
		memset(&buffs[0].at(ch).front(), 0, sizeof(samp_type) * buffs[0].at(ch).size()); // let's zero it for debugging purposes
		// Reserve+resize buff2
		buffs[1].at(ch).reserve(rx_rate);
		buffs[1].at(ch).resize(rx_rate);
		memset(&buffs[1].at(ch).front(), 0, sizeof(samp_type) * buffs[1].at(ch).size()); // let's zero it for debugging purposes
	}
	// Now create the pointer vectors, one for current, one for waiting
	std::vector<samp_type*> buff_ptrs[2];
	buff_ptrs[0].resize(channel_nums.size());
	buff_ptrs[1].resize(channel_nums.size()); // we will be writing the pointers during the loop, not here

	int tIdx = 0; // used for buffer index, 0 or 1
	int bufIdx = 0; // used to index into the vector

    bool overflow_message = true;

    // setup streaming
    uhd::stream_cmd_t stream_cmd((num_requested_samples == 0)
                                     ? uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS
                                     : uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);
    stream_cmd.num_samps  = size_t(num_requested_samples);
    stream_cmd.stream_now = false; // don't do immediately but sync to the second
	uhd::time_spec_t time2send(usrp->get_time_now().get_full_secs() + 2, 0.0); // use time_now instead of pps?
    stream_cmd.time_spec = time2send;
	printf("Streaming will start at %lld\n", time2send.get_full_secs());
    rx_stream->issue_stream_cmd(stream_cmd);

    typedef std::map<size_t, size_t> SizeMap;
    SizeMap mapSizes;
    const auto start_time = std::chrono::steady_clock::now();
    const auto stop_time =
        start_time + std::chrono::milliseconds(int64_t(1000 * time_requested));
    // Track time and samps between updating the BW summary
    auto last_update                     = start_time;
    unsigned long long last_update_samps = 0;

	// metadata holding
	uhd::time_spec_t rxtime;
	
	// counter of numFiles so far
	int64_t numFilesWritten = 0;
	
    // Run this loop until either time expired (if a duration was given), until
    // the requested number of samples were collected (if such a number was
    // given), or until Ctrl-C was pressed.
    while (not stop_signal_called
           and (num_requested_samples != num_total_samps or num_requested_samples == 0)
           and (time_requested == 0.0 or std::chrono::steady_clock::now() <= stop_time)) {
        const auto now = std::chrono::steady_clock::now();

		// write the vector of pointers before receiving (only need to write the buff_ptrs at tIdx)	
		for (size_t i = 0; i < channel_nums.size(); i++){
			buff_ptrs[tIdx].at(i) = &buffs[tIdx].at(i).at(bufIdx);
		}
		// perform the receive
        size_t num_rx_samps =
            // rx_stream->recv(&buff[tIdx].at(bufIdx), samps_per_buff, md, 3.0, enable_size_map); // we edit to write at bufIdx
			rx_stream->recv(buff_ptrs[tIdx], samps_per_buff, md, 3.0, enable_size_map); // we edit to write at bufIdx
		
		// =========== read the metadata
		// std::cout << md.to_pp_string(false) << std::endl;
		rxtime = md.time_spec;
//		printf("Sec : %lld, frac sec : %.11f, samples: %zd\n", rxtime.get_full_secs(), rxtime.get_frac_secs(), num_rx_samps);
//		printf("Tick Count: %ld. \n", rxtime.get_tick_count(200e6));
//        uhd::time_spec_t timenow = usrp->get_time_now();
//        printf("Time from USRP: %lld, frac sec: %.11f\n", timenow.get_full_secs(), timenow.get_frac_secs()); // this line shows that the TwinRX appears to be doubling the duration (maybe because two DSPs and they added by accident? either way, using the metadata will be incorrect for the twinRX)
		// ============================
		
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
                           % (usrp->get_rx_rate(channel_nums[0]) * sizeof(samp_type) / 1e6);
            }
            // continue;
			break; // we want to ensure timing integrity, so if overflow let's end
        }
        if (md.error_code != uhd::rx_metadata_t::ERROR_CODE_NONE) {
            std::string error = str(boost::format("Receiver error: %s") % md.strerror());
//            if (continue_on_bad_packet) {
//                std::cerr << error << std::endl;
//                continue;
//            } else
//                throw std::runtime_error(error);
            break; // again, ensure timing integrity, always break
        }

        if (enable_size_map) {
            SizeMap::iterator it = mapSizes.find(num_rx_samps);
            if (it == mapSizes.end())
                mapSizes[num_rx_samps] = 0;
            mapSizes[num_rx_samps] += 1;
        }

        num_total_samps += num_rx_samps;

		// ============ check buffers
		if (verbose) {printf("Buf: %d. W: %d\n", tIdx, bufIdx);}
		// update the new idx to write to
		bufIdx = bufIdx + num_rx_samps;
		if (bufIdx == rx_rate) // then move to next buffer
		{
			// start thread to write current buffer, for each subfolder
			for (int i = 0; i < folders.size(); i++){
//				std::thread t(save_to_file<samp_type>, std::ref(folders.at(i)), rxtime.get_full_secs(), std::ref(buffs[tIdx].at(i)), threshold); // since rxtime is not accurate for twinRX, we revert to just a plain counter based on start timing
                std::thread t(save_to_file<samp_type>, std::ref(folders.at(i)), time2send.get_full_secs() + numFilesWritten, std::ref(buffs[tIdx].at(i)), threshold, saturation_warning); 

				t.detach();
			}
			
			// update indices
			tIdx = (tIdx + 1) % 2;
			bufIdx = 0;
			
			numFilesWritten += 1;
		}
		// ==========================
		
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

typedef std::function<uhd::sensor_value_t(const std::string&)> get_sensor_fn_t;

bool check_locked_sensor(std::vector<std::string> sensor_names,
    const char* sensor_name,
    get_sensor_fn_t get_sensor_fn,
    double setup_time)
{
    if (std::find(sensor_names.begin(), sensor_names.end(), sensor_name)
        == sensor_names.end())
        return false;

    auto setup_timeout = std::chrono::steady_clock::now()
                         + std::chrono::milliseconds(int64_t(setup_time * 1000));
    bool lock_detected = false;

    std::cout << boost::format("Waiting for \"%s\": ") % sensor_name;
    std::cout.flush();

    while (true) {
        if (lock_detected and (std::chrono::steady_clock::now() > setup_timeout)) {
            std::cout << " locked." << std::endl;
            break;
        }
        if (get_sensor_fn(sensor_name).to_bool()) {
            std::cout << "+";
            std::cout.flush();
            lock_detected = true;
        } else {
            if (std::chrono::steady_clock::now() > setup_timeout) {
                std::cout << std::endl;
                throw std::runtime_error(
                    str(boost::format(
                            "timed out waiting for consecutive locks on sensor \"%s\"")
                        % sensor_name));
            }
            std::cout << "_";
            std::cout.flush();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << std::endl;
    return true;
}

int UHD_SAFE_MAIN(int argc, char* argv[])
{
    // variables to be set by po
    std::string args, file, type, ant, subdev, ref, wirefmt, folder;
	std::string channel_list, ant_list;
    std::string freqstr_list;
    size_t channel, total_num_samps, spb;
    double rate, freq, gain, bw, total_time, setup_time, lo_offset;
    double threshold, saturation_warning;

    // setup the program options
    po::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "multi uhd device address args")
        // ("file", po::value<std::string>(&file)->default_value("usrp_samples.dat"), "name of the file to write binary samples to")
        ("type", po::value<std::string>(&type)->default_value("short"), "sample type: double, float, short, or byte")
        ("nsamps", po::value<size_t>(&total_num_samps)->default_value(0), "total number of samples to receive")
        ("duration", po::value<double>(&total_time)->default_value(0), "total number of seconds to receive")
        ("time", po::value<double>(&total_time), "(DEPRECATED) will go away soon! Use --duration instead")
        ("spb", po::value<size_t>(&spb)->default_value(10000), "samples per buffer")
        ("rate", po::value<double>(&rate)->default_value(1e6), "rate of incoming samples")
        // ("freq", po::value<double>(&freq)->default_value(0.0), "RF center frequency in Hz")
        ("freq", po::value<std::string>(&freqstr_list)->default_value("0.0"), "RF center frequency in Hz")
        ("lo-offset", po::value<double>(&lo_offset)->default_value(0.0),
            "Offset for frontend LO in Hz (optional)")
        ("gain", po::value<double>(&gain), "gain for the RF chain")
        // ("ant", po::value<std::string>(&ant), "antenna selection")
		("ants", po::value<std::string>(&ant_list), "antenna selection, use in conjunction with --channels to specify ports: \"TX/RX\" or \"RX2\"(default)")
        ("subdev", po::value<std::string>(&subdev), "subdevice specification")
        // ("channel", po::value<size_t>(&channel)->default_value(0), "which channel to use")
		("channels", po::value<std::string>(&channel_list)->default_value("0"), "which channel(s) to use (specify \"0\", \"1\", \"0,1\", etc)")
        ("bw", po::value<double>(&bw), "analog frontend filter bandwidth in Hz")
        ("ref", po::value<std::string>(&ref)->default_value("internal"), "reference source (internal, gpsdo, external, mimo)")
        ("wirefmt", po::value<std::string>(&wirefmt)->default_value("sc16"), "wire format (sc8, sc16 or s16)")
        ("setup", po::value<double>(&setup_time)->default_value(1.0), "seconds of setup time")
        ("progress", "periodically display short-term bandwidth")
        ("stats", "show average bandwidth on exit")
        ("sizemap", "track packet size and display breakdown on exit")
        ("null", "run without writing to file")
        ("continue", "don't abort on a bad packet")
        ("skip-lo", "skip checking LO lock status")
        ("int-n", "tune USRP with integer-N tuning")
        ("verbose", "turn on verbose reporting")
		("folder", po::value<std::string>(&folder)->default_value(""), "path to write files to (will be created if it doesn't exist)")
        ("threshold", po::value<double>(&threshold)->default_value(0), "amplitude threshold before writing to disk")
        ("saturation-warning", po::value<double>(&saturation_warning)->default_value(0), "threshold value for saturation warning (any sample whose abs value exceeds this will produce a warning)")
    ;
	
	// Wizard style for clueless users
    if (argc == 1)
    {
        std::cout << "*** Wizard Mode ***" << std::endl;
        std::cout << "If you would like to specify arguments yourself, please use --help to see the possible options." << std::endl;
        std::cout << "This mode will only iterate through the common options." << std::endl;

        int wiz_fs;
        std::cout << "Sample rate: ";
        std::cin >> wiz_fs;

        double wiz_freq;
        std::cout << "Centre frequency (Hz): ";
        std::cin >> wiz_freq;

        double wiz_gain;
        std::cout << "Gain (dB): ";
        std::cin >> wiz_gain;

        std::string wiz_folder;
        std::cout << "Output folder: ";
        std::cin >> wiz_folder;

        // TODO: Format argc and argv appropriately 
    }
	
	
    // clang-format on
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // print the help message
    if (vm.count("help")) {
        std::cout << boost::format("UHD RX samples to file %s") % desc << std::endl;
        std::cout << std::endl
                  << "This application streams data from multiple channels of a USRP "
                     "device to multiple 1 second files. Combination examples:\n"
					 "--channels 0,1 --ants TX/RX,RX2 \n"
					 "will use channel 0 (usually left half of USRP) with TX/RX port, and channel 1 (right half) with RX2 port.\n"
					 "If --ants is not specified, all channels default to the RX2 port.\n"
					 "For TwinRXs, the subdev must be specified. Typically, you can just add --subdev \"A:0 A:1 B:0 B:1\"\n"
					 "and then assign the channels you would like. For example,\n"
					 "To record on RF A:RX1 and RF B:RX 2, specify --subdev \"A:0 A:1 B:0 B:1\" --channels 0,3 --ants RX1,RX2\n"
 					 "To record on RF A:RX2 and RF B:RX 2, specify --subdev \"A:0 A:1 B:0 B:1\" --channels 1,3 --ants RX2,RX2\n"
 					 "To record on RF A:RX1 and RF A:RX 2, specify --subdev \"A:0 A:1 B:0 B:1\" --channels 0,1 --ants RX1,RX2\n"
					 "To record on all 4 ports, specify --subdev \"A:0 A:1 B:0 B:1\" --channels 0,1,2,3 --ants RX1,RX2,RX1,RX2\n"
					 "In theory, the subdev specifies the available DSPs, A:0 A:1 corresponding to the first daughterboard and \n"
					 "B:0 B:1 corresponding to the second daughterboard. The channels argument then references the subdevs available.\n"
					 "Hence, one can also record on RF A:RX2, RF B:RX2 by doing\n"
					 "--subdev \"A:0 B:0\" --channels 0,1 --ants RX2,RX2\n"
					 "It is up to you to choose how you want to specify the arguments. They should make no difference in the end.\n"
                     "Channel-specific centre frequencies may be set in a similar comma-delimited manner. For example,"
                     "we can set --channels 0,2 --freq 1.5e9,1.6e9 to use 1.5GHz for channel 0 and 1.6GHz for channel 2."
                  << std::endl;
        return ~0;
    }

    // create outer directory if needed
    boost::filesystem::create_directories(folder.c_str());
    // check if it exists
    if (boost::filesystem::is_directory(folder.c_str())) {
        std::cout << "Data folder at " << folder << " has been created/exists. Proceeding..." << std::endl;
    }
    else {
        std::cout << "Failed to create folder at " << folder << ", exiting!" << std::endl;
        return 1;
    }
    // write a log file
    std::string logpath = folder + pathsplit + "log.txt";
    FILE *flog = fopen(logpath.c_str(), "w");
    for (int i = 1; i < argc; i++){
        fprintf(flog, "%s ",  argv[i]);
    }
    fprintf(flog, "\n");
    fclose(flog);

    bool bw_summary             = vm.count("progress") > 0;
    bool stats                  = vm.count("stats") > 0;
    bool null                   = vm.count("null") > 0;
    bool enable_size_map        = vm.count("sizemap") > 0;
    bool continue_on_bad_packet = vm.count("continue") > 0;
    bool verbose                = vm.count("verbose") > 0;

    if (enable_size_map)
        std::cout << "Packet size tracking enabled - will only recv one packet at a time!"
                  << std::endl;

    // create a usrp device
    std::cout << std::endl;
    std::cout << boost::format("Creating the usrp device with: %s...") % args
              << std::endl;
    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);

    // Lock mboard clocks
    if (vm.count("ref")) {
        usrp->set_clock_source(ref);
        usrp->set_time_source(ref);
    }

    // always select the subdevice first, the channel mapping affects the other settings
    if (vm.count("subdev"))
        usrp->set_rx_subdev_spec(subdev);

    std::cout << boost::format("Using Device: %s") % usrp->get_pp_string() << std::endl;

	// pre-allocate some vectors
	std::vector<std::string> folders;
	// detect which channels to use
    std::vector<std::string> channel_strings;
    std::vector<size_t> channel_nums;
    boost::split(channel_strings, channel_list, boost::is_any_of("\"',"));
	// check that antenna specification matches channel specification (in terms of length)
	std::vector<std::string> ant_strings;
	if (vm.count("ants")){
		boost::split(ant_strings, ant_list, boost::is_any_of("\"',"));
		if (ant_strings.size() != channel_strings.size()){
			throw std::runtime_error("Number of antennas specified does not correspond to number of channels specified. Either match the number or leave antenna specification blank to use RX2 for all channels.");
		}
	}

    // for channel-specific centre frequencies
    std::vector<std::string> freq_strings;
    std::vector<double> freq_vals;
    boost::split(freq_strings, freqstr_list, boost::is_any_of("\"',"));
    // ensure there's as many freq centres as there are channels specified
    if (freq_strings.size() != channel_strings.size())
        throw std::runtime_error("Number of centre frequencies does not correspond to number of channels specified.");
	
	// loop over the channels specified
    for (size_t ch = 0; ch < channel_strings.size(); ch++) {
        size_t channel = std::stoi(channel_strings[ch]);
        if (channel >= usrp->get_rx_num_channels()) {
            throw std::runtime_error("Invalid channel(s) specified.");
        } else{
            channel_nums.push_back(channel);
		}

        // extract the frequency value and append to the vector for storage
        freq = std::stod(freq_strings.at(ch));
        freq_vals.push_back(freq);
		
		// everything else is inside this loop, since they reference a channel
		// set the antenna for each channel
		if (vm.count("ants")){
			usrp->set_rx_antenna(ant_strings[ch], channel);
		}
		std::cout << "Using channel " << channel_strings[ch] << " with antenna " << usrp->get_rx_antenna(channel) << std::endl;
		
		// create a subfolder for the channel
		std::string subfolder = folder + pathsplit + channel_strings[ch];
		folders.push_back(subfolder);
		boost::filesystem::create_directories(subfolder);
		// check if it exists
		if (boost::filesystem::is_directory(subfolder.c_str())) {
			std::cout << "Data folder at " << subfolder << " has been created/exists. Proceeding..." << std::endl;
		}
		
		// set the sample rate for each channel
		if (rate <= 0.0) {
			std::cerr << "Please specify a valid sample rate" << std::endl;
			return ~0;
		}
		std::cout << boost::format("Setting RX Rate: %f Msps...") % (rate / 1e6) << std::endl;
		usrp->set_rx_rate(rate, channel);
		std::cout << boost::format("Actual RX Rate: %f Msps...")
						 % (usrp->get_rx_rate(channel) / 1e6)
				  << std::endl
				  << std::endl;
				  
		// set the center frequency
		if (vm.count("freq")) { // with default of 0.0 this will always be true
			std::cout << boost::format("Setting RX Freq: %f MHz...") % (freq / 1e6)
					  << std::endl;
			std::cout << boost::format("Setting RX LO Offset: %f MHz...") % (lo_offset / 1e6)
					  << std::endl;
			uhd::tune_request_t tune_request(freq, lo_offset);
			if (vm.count("int-n"))
				tune_request.args = uhd::device_addr_t("mode_n=integer");
			usrp->set_rx_freq(tune_request, channel);
			std::cout << boost::format("Actual RX Freq: %f MHz...")
							 % (usrp->get_rx_freq(channel) / 1e6)
					  << std::endl
					  << std::endl;
		}
		
		// set the rf gain
		if (vm.count("gain")) {
			std::cout << boost::format("Setting RX Gain: %f dB...") % gain << std::endl;
			usrp->set_rx_gain(gain, channel);
			std::cout << boost::format("Actual RX Gain: %f dB...")
							 % usrp->get_rx_gain(channel)
					  << std::endl
					  << std::endl;
		}

		// set the IF filter bandwidth
		if (vm.count("bw")) {
			std::cout << boost::format("Setting RX Bandwidth: %f MHz...") % (bw / 1e6)
					  << std::endl;
			usrp->set_rx_bandwidth(bw, channel);
			std::cout << boost::format("Actual RX Bandwidth: %f MHz...")
							 % (usrp->get_rx_bandwidth(channel) / 1e6)
					  << std::endl
					  << std::endl;
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(1000 * setup_time)));

		// check Ref and LO Lock detect
		if (not vm.count("skip-lo")) {
			check_locked_sensor(usrp->get_rx_sensor_names(channel),
				"lo_locked",
				[usrp, channel](const std::string& sensor_name) {
					return usrp->get_rx_sensor(sensor_name, channel);
				},
				setup_time);
			if (ref == "mimo") {
				check_locked_sensor(usrp->get_mboard_sensor_names(0),
					"mimo_locked",
					[usrp](const std::string& sensor_name) {
						return usrp->get_mboard_sensor(sensor_name);
					},
					setup_time);
			}
			if (ref == "external") {
				check_locked_sensor(usrp->get_mboard_sensor_names(0),
					"ref_locked",
					[usrp](const std::string& sensor_name) {
						return usrp->get_mboard_sensor(sensor_name);
					},
					setup_time);
			}
		}
		
    } // end of channel loop

	// check that samples per buffer is a divisor of sample rate
	if (static_cast<int>(rate) % spb != 0)
	{
		std::cout << "========= Make sure SPB is a divisor of sample rate. Exiting." << std::endl;
		return -1;
	}

    if (total_num_samps == 0) {
        std::signal(SIGINT, &sig_int_handler);
        std::cout << "Press Ctrl + C to stop streaming..." << std::endl;
    }
	
	

#define recv_to_file_args(format) \
    (usrp,                        \
        format,                   \
        wirefmt,                  \
        channel_nums,             \
        spb,                      \
        total_num_samps,          \
        folders,                  \
        threshold,                \
        saturation_warning,       \
        total_time,               \
        bw_summary,               \
        stats,                    \
        null,                     \
        enable_size_map,          \
        verbose)
    // recv to file
    
    do{
        if (wirefmt == "s16") {
            if (type == "double")
                recv_to_file<double> recv_to_file_args("f64");
            else if (type == "float")
                recv_to_file<float> recv_to_file_args("f32");
            else if (type == "short")
                recv_to_file<short> recv_to_file_args("s16");
            else
                throw std::runtime_error("Unknown type " + type);
        } else {
            if (type == "double")
                recv_to_file<std::complex<double>> recv_to_file_args("fc64");
            else if (type == "float")
                recv_to_file<std::complex<float>> recv_to_file_args("fc32");
            else if (type == "short")
                recv_to_file<std::complex<short>> recv_to_file_args("sc16");
            else if (type == "byte")
                recv_to_file<std::complex<char>> recv_to_file_args("sc8");
            else
                throw std::runtime_error("Unknown type " + type);
        }
    }while(continue_on_bad_packet);

    // finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;

    return EXIT_SUCCESS;
}
