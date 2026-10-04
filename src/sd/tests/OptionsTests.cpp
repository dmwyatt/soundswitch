#include <doctest/doctest.h>
#include "Options.h"

using namespace ap;

TEST_CASE("ParseOptions with no arguments lists every device without IDs")
{
	const Options options = ParseOptions({});

	CHECK(options.Filter == DeviceFilter());
	CHECK_FALSE(options.ShowIds);
	CHECK_FALSE(options.ShowHelp);
}

TEST_CASE("ParseOptions turns on IDs for -id")
{
	CHECK(ParseOptions({ "-id" }).ShowIds);
}

TEST_CASE("ParseOptions asks for help for -h and --help")
{
	CHECK(ParseOptions({ "-h" }).ShowHelp);
	CHECK(ParseOptions({ "--help" }).ShowHelp);
}

TEST_CASE("ParseOptions keeps only the states named after -f, in both flows")
{
	const Options options = ParseOptions({ "-f", "active", "unplugged" });

	CHECK(options.Filter.States == std::set{ DeviceState::Active, DeviceState::Unplugged });
	CHECK(options.Filter.Flows == DeviceFilter().Flows);
}

TEST_CASE("ParseOptions keeps only the flows named after -f, in every state")
{
	const Options options = ParseOptions({ "-f", "capture" });

	CHECK(options.Filter.States == DeviceFilter().States);
	CHECK(options.Filter.Flows == std::set{ DataFlow::Capture });
}

TEST_CASE("ParseOptions understands every filter word")
{
	const Options options = ParseOptions({ "-f", "active", "disabled", "notpresent", "unplugged", "capture", "render" });

	CHECK(options.Filter == DeviceFilter());
}

TEST_CASE("ParseOptions lists everything for -f with no words")
{
	CHECK(ParseOptions({ "-f" }).Filter == DeviceFilter());
}

TEST_CASE("ParseOptions ends the filter words at the next option")
{
	const Options options = ParseOptions({ "-f", "disabled", "render", "-id" });

	CHECK(options.Filter.States == std::set{ DeviceState::Disabled });
	CHECK(options.Filter.Flows == std::set{ DataFlow::Render });
	CHECK(options.ShowIds);
}

TEST_CASE("ParseOptions lets a later -f replace an earlier one")
{
	const Options options = ParseOptions({ "-f", "active", "render", "-f", "disabled" });

	CHECK(options.Filter.States == std::set{ DeviceState::Disabled });
	CHECK(options.Filter.Flows == DeviceFilter().Flows);
}

TEST_CASE("ParseOptions rejects a filter word it does not know")
{
	CHECK_THROWS_WITH_AS(ParseOptions({ "-f", "actve" }), "Unknown filter 'actve'", UsageError);
}

TEST_CASE("ParseOptions rejects an option it does not know")
{
	CHECK_THROWS_WITH_AS(ParseOptions({ "-x" }), "Unknown argument '-x'", UsageError);
}

TEST_CASE("ParseOptions rejects a word that does not follow -f")
{
	CHECK_THROWS_WITH_AS(ParseOptions({ "active" }), "Unknown argument 'active'", UsageError);
	CHECK_THROWS_WITH_AS(ParseOptions({ "-id", "active" }), "Unknown argument 'active'", UsageError);
}
