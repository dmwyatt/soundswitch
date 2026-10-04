#include "Options.h"

#include <algorithm>
#include <map>

namespace ap
{
	namespace
	{
		const std::map<std::string, DeviceState> STATE_WORDS = {
			{ "active", DeviceState::Active },
			{ "disabled", DeviceState::Disabled },
			{ "notpresent", DeviceState::NotPresent },
			{ "unplugged", DeviceState::Unplugged } };

		const std::map<std::string, DataFlow> FLOW_WORDS = {
			{ "capture", DataFlow::Capture },
			{ "render", DataFlow::Render } };

		bool IsOption(const std::string& arg)
		{
			return arg.starts_with('-');
		}

		// Words of one kind narrow that kind; a kind with no words stays unfiltered.
		DeviceFilter ParseFilter(const std::vector<std::string>& words)
		{
			std::set<DeviceState> states;
			std::set<DataFlow> flows;
			for(const std::string& word : words)
			{
				if(const auto state = STATE_WORDS.find(word); state != STATE_WORDS.end()) states.insert(state->second);
				else if(const auto flow = FLOW_WORDS.find(word); flow != FLOW_WORDS.end()) flows.insert(flow->second);
				else throw UsageError("Unknown filter '" + word + "'");
			}

			DeviceFilter filter;
			if(!states.empty()) filter.States = states;
			if(!flows.empty()) filter.Flows = flows;
			return filter;
		}
	}

	const char* const USAGE =
		"Usage: sd [-f FILTER...] [-id]\n"
		"\n"
		"Lists audio devices, one per line:\n"
		"  [ID,]NAME,DESCRIPTION,FLOW,STATE[,ROLES]\n"
		"\n"
		"NAME is the audio adapter and DESCRIPTION is the device on it. FLOW is\n"
		"Render or Capture. ROLES are the roles the device is the default device\n"
		"for (Console, Multimedia, Communications), joined with \"|\".\n"
		"\n"
		"Options:\n"
		"  -f FILTER...  List only matching devices. State filters: active,\n"
		"                disabled, notpresent, unplugged. Flow filters: capture,\n"
		"                render. Without a state filter every state is listed;\n"
		"                without a flow filter both flows are.\n"
		"  -id           Start each line with the device ID.\n"
		"  -h, --help    Show this help.\n";

	Options ParseOptions(const std::vector<std::string>& args)
	{
		Options options;
		auto arg = args.begin();
		while(arg != args.end())
		{
			if(*arg == "-f")
			{
				const auto wordsEnd = std::find_if(arg + 1, args.end(), IsOption);
				options.Filter = ParseFilter({ arg + 1, wordsEnd });
				arg = wordsEnd;
				continue;
			}

			if(*arg == "-id") options.ShowIds = true;
			else if(*arg == "-h" || *arg == "--help") options.ShowHelp = true;
			else throw UsageError("Unknown argument '" + *arg + "'");
			++arg;
		}
		return options;
	}
}
