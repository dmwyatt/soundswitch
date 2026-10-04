#include "Stringify.h"
#include "Device.h"

using namespace ap;
using namespace std::string_literals;

namespace
{
	Device Speakers()
	{
		return {
			.Id = L"{speakers}",
			.Name = L"Realtek Audio",
			.Description = L"Speakers",
			.Flow = DataFlow::Render,
			.State = DeviceState::Active };
	}
}

TEST_CASE("RolesFor marks each role whose default device has this ID")
{
	const DefaultDevices defaults = { .Console = L"a", .Multimedia = L"b", .Communications = L"a" };

	const DeviceRoles roles = RolesFor(L"a", defaults);

	CHECK(roles.Console);
	CHECK_FALSE(roles.Multimedia);
	CHECK(roles.Communications);
}

TEST_CASE("RolesFor gives no roles when no role has a default device")
{
	const DeviceRoles roles = RolesFor(L"a", DefaultDevices());

	CHECK_FALSE(roles.Console);
	CHECK_FALSE(roles.Multimedia);
	CHECK_FALSE(roles.Communications);
}

TEST_CASE("ListedBefore puts a render device before a capture device whatever their IDs")
{
	const Device render = { .Id = L"z", .Flow = DataFlow::Render };
	const Device capture = { .Id = L"a", .Flow = DataFlow::Capture };

	CHECK(ListedBefore(render, capture));
	CHECK_FALSE(ListedBefore(capture, render));
}

TEST_CASE("ListedBefore orders devices of one flow by ID")
{
	const Device first = { .Id = L"a", .Flow = DataFlow::Capture };
	const Device second = { .Id = L"b", .Flow = DataFlow::Capture };

	CHECK(ListedBefore(first, second));
	CHECK_FALSE(ListedBefore(second, first));
	CHECK_FALSE(ListedBefore(first, first));
}

TEST_CASE("FormatDevice joins name, description, flow and state with commas")
{
	CHECK(FormatDevice(Speakers(), false) == L"Realtek Audio,Speakers,Render,Active"s);
}

TEST_CASE("FormatDevice starts with the ID when asked")
{
	CHECK(FormatDevice(Speakers(), true) == L"{speakers},Realtek Audio,Speakers,Render,Active"s);
}

TEST_CASE("FormatDevice names the flow and each state")
{
	Device device = Speakers();
	device.Flow = DataFlow::Capture;

	device.State = DeviceState::Disabled;
	CHECK(FormatDevice(device, false) == L"Realtek Audio,Speakers,Capture,Disabled"s);
	device.State = DeviceState::NotPresent;
	CHECK(FormatDevice(device, false) == L"Realtek Audio,Speakers,Capture,Not present"s);
	device.State = DeviceState::Unplugged;
	CHECK(FormatDevice(device, false) == L"Realtek Audio,Speakers,Capture,Unplugged"s);
}

TEST_CASE("FormatDevice appends the device's roles joined with pipes")
{
	Device device = Speakers();

	device.Roles = { .Console = true, .Multimedia = true, .Communications = true };
	CHECK(FormatDevice(device, false) == L"Realtek Audio,Speakers,Render,Active,Console|Multimedia|Communications"s);
	device.Roles = { .Communications = true };
	CHECK(FormatDevice(device, false) == L"Realtek Audio,Speakers,Render,Active,Communications"s);
}

TEST_CASE("FormatDevice shows [unknown] for a missing name, description or state")
{
	const Device device = { .Id = L"{speakers}", .Flow = DataFlow::Render };

	CHECK(FormatDevice(device, false) == L"[unknown],[unknown],Render,[unknown]"s);
}

TEST_CASE("FormatDevice quotes a field that contains a comma")
{
	Device device = Speakers();
	device.Description = L"Speakers (Realtek, Front)";

	CHECK(FormatDevice(device, false) == L"Realtek Audio,\"Speakers (Realtek, Front)\",Render,Active"s);
}

TEST_CASE("FormatDevice quotes a field that contains a quote, doubling the quote")
{
	Device device = Speakers();
	device.Name = L"The \"Good\" Card";

	CHECK(FormatDevice(device, false) == L"\"The \"\"Good\"\" Card\",Speakers,Render,Active"s);
}

TEST_CASE("FormatDevice replaces line breaks with spaces so a device cannot span lines")
{
	Device device = Speakers();
	device.Id = L"first\r\nsecond";
	device.Name = L"Realtek Audio\nFake Adapter,Fake Device,Render,Active";
	device.Description = L"Left Right End";

	CHECK(FormatDevice(device, true) == L"first  second,\"Realtek Audio Fake Adapter,Fake Device,Render,Active\",Left Right End,Render,Active"s);
}

TEST_CASE("FormatDevice replaces control characters with spaces so a device cannot drive the terminal")
{
	Device device = Speakers();
	device.Name = L"Red\x1b[31m";
	device.Description = L"Tab\tBell\aDel\x7fNext\u0085";

	CHECK(FormatDevice(device, false) == L"Red [31m,Tab Bell Del Next ,Render,Active"s);
}
