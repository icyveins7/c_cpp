#include "simpleUsrpSetups.h"
#include <uhd/exception.hpp>
#include <uhd/types/tune_request.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/static.hpp>
#include <uhd/utils/thread.hpp>


int main()
{
	struct SettingsUSRP rxsettings;
	//rxsettings.subdev_args = "A:A"; // the default usually enables everything

	ChannelSetting globalsetting;
	globalsetting.rate = 240e3;
	globalsetting.freq = 100e6;

	rxsettings.chnlsettings.push_back(globalsetting);

	printf("Setting up..");
	uhd::usrp::multi_usrp::sptr rx_usrp = setupRxUSRP(rxsettings);
	printf("Done.\n");

	printf("%s\n", rx_usrp->get_pp_string().c_str());

	// Get settings back
	struct SettingsUSRP getsettings = checkRxSettings(rx_usrp);
	printf(
		"Clocksrc = %s\n"
		"Timesrc = %s\n"
		"Subdev = %s\n",
		getsettings.clock_source.c_str(),
		getsettings.time_source.c_str(),
		getsettings.subdev_args.c_str()
	);
	for (int i = 0; i < getsettings.chnlsettings.size(); i++)
	{
		printf(
			"Channel %zd:\n"
			"Rate: %f \n"
			"Freq: %f \n"
			"Gain: %f \n"
			"BW : %f \n"
			"Antenna: %s\n",
			getsettings.chnlsettings.at(i).channel,
			getsettings.chnlsettings.at(i).rate,
			getsettings.chnlsettings.at(i).freq,
			getsettings.chnlsettings.at(i).gain,
			getsettings.chnlsettings.at(i).bw,
			getsettings.chnlsettings.at(i).ant.c_str()
		);
	}

	// Check some other things with the usrp object



	return 0;
}