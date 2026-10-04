#include "Stringify.h"
#include "FakeEndpoints.h"
#include "Endpoints.h"

#include <system_error>

using namespace ap;
using namespace ap::fakes;
using namespace std::string_literals;

namespace
{
	const DWORD UNRECOGNIZED_STATE = 0x100;

	ULONG ReferenceCount(IUnknown& object)
	{
		object.AddRef();
		return object.Release();
	}
}

TEST_CASE("ReadDevices reads a device's ID, name, description, flow and state")
{
	const ComPtr<FakeEnumerator> enumerator = Make<FakeEnumerator>();
	enumerator->Add({ .Id = L"mic", .Flow = eCapture, .State = DEVICE_STATE_UNPLUGGED, .Name = L"USB Audio", .Description = L"Microphone" });

	const std::vector<Device> devices = ReadDevices(*enumerator.Get(), DeviceFilter());

	REQUIRE(devices.size() == 1);
	CHECK(devices[0].Id == L"mic"s);
	CHECK(devices[0].Name == L"USB Audio"s);
	CHECK(devices[0].Description == L"Microphone"s);
	CHECK(devices[0].Flow == DataFlow::Capture);
	CHECK(devices[0].State == DeviceState::Unplugged);
}

TEST_CASE("ReadDevices reads only the flows in the filter")
{
	const ComPtr<FakeEnumerator> enumerator = Make<FakeEnumerator>();
	enumerator->Add({ .Id = L"speakers", .Flow = eRender });
	enumerator->Add({ .Id = L"mic", .Flow = eCapture });
	DeviceFilter filter;
	filter.Flows = { DataFlow::Capture };

	const std::vector<Device> devices = ReadDevices(*enumerator.Get(), filter);

	REQUIRE(devices.size() == 1);
	CHECK(devices[0].Id == L"mic"s);
}

TEST_CASE("ReadDevices reads only the states in the filter")
{
	const ComPtr<FakeEnumerator> enumerator = Make<FakeEnumerator>();
	enumerator->Add({ .Id = L"active", .State = DEVICE_STATE_ACTIVE });
	enumerator->Add({ .Id = L"disabled", .State = DEVICE_STATE_DISABLED });
	enumerator->Add({ .Id = L"notpresent", .State = DEVICE_STATE_NOTPRESENT });
	enumerator->Add({ .Id = L"unplugged", .State = DEVICE_STATE_UNPLUGGED });
	DeviceFilter filter;
	filter.States = { DeviceState::Disabled, DeviceState::NotPresent };
	filter.Flows = { DataFlow::Render };

	const std::vector<Device> devices = ReadDevices(*enumerator.Get(), filter);

	REQUIRE(devices.size() == 2);
	CHECK(devices[0].Id == L"disabled"s);
	CHECK(devices[0].State == DeviceState::Disabled);
	CHECK(devices[1].Id == L"notpresent"s);
	CHECK(devices[1].State == DeviceState::NotPresent);
}

TEST_CASE("ReadDevices gives a default device the roles it is the default for, within its own flow")
{
	const ComPtr<FakeEnumerator> enumerator = Make<FakeEnumerator>();
	const ComPtr<FakeEndpoint> speakers = enumerator->Add({ .Id = L"speakers", .Flow = eRender });
	const ComPtr<FakeEndpoint> headset = enumerator->Add({ .Id = L"headset", .Flow = eRender });
	enumerator->Add({ .Id = L"mic", .Flow = eCapture });
	enumerator->Defaults = {
		{ { eRender, eConsole }, speakers },
		{ { eRender, eMultimedia }, speakers },
		{ { eRender, eCommunications }, headset } };

	const std::vector<Device> devices = ReadDevices(*enumerator.Get(), DeviceFilter());

	REQUIRE(devices.size() == 3);
	CHECK(devices[0].Roles.Console);
	CHECK(devices[0].Roles.Multimedia);
	CHECK_FALSE(devices[0].Roles.Communications);
	CHECK_FALSE(devices[1].Roles.Console);
	CHECK_FALSE(devices[1].Roles.Multimedia);
	CHECK(devices[1].Roles.Communications);
	CHECK_FALSE(devices[2].Roles.Console);
	CHECK_FALSE(devices[2].Roles.Multimedia);
	CHECK_FALSE(devices[2].Roles.Communications);
}

TEST_CASE("ReadDevices lists the devices of a flow that has no default device")
{
	const ComPtr<FakeEnumerator> enumerator = Make<FakeEnumerator>();
	enumerator->Add({ .Id = L"mic", .Flow = eCapture, .State = DEVICE_STATE_UNPLUGGED });

	const std::vector<Device> devices = ReadDevices(*enumerator.Get(), DeviceFilter());

	REQUIRE(devices.size() == 1);
	CHECK(devices[0].Id == L"mic"s);
	CHECK_FALSE(devices[0].Roles.Console);
	CHECK_FALSE(devices[0].Roles.Multimedia);
	CHECK_FALSE(devices[0].Roles.Communications);
}

TEST_CASE("ReadDevices leaves the name and description empty when the device has neither")
{
	const ComPtr<FakeEnumerator> enumerator = Make<FakeEnumerator>();
	enumerator->Add({ .Id = L"speakers", .Name = std::nullopt, .Description = std::nullopt });

	const std::vector<Device> devices = ReadDevices(*enumerator.Get(), DeviceFilter());

	REQUIRE(devices.size() == 1);
	CHECK_FALSE(devices[0].Name.has_value());
	CHECK_FALSE(devices[0].Description.has_value());
}

TEST_CASE("ReadDevices leaves the state empty when Windows reports one it does not know")
{
	const ComPtr<FakeEnumerator> enumerator = Make<FakeEnumerator>();
	enumerator->Add({ .Id = L"speakers", .State = DEVICE_STATE_ACTIVE | UNRECOGNIZED_STATE });

	const std::vector<Device> devices = ReadDevices(*enumerator.Get(), DeviceFilter());

	REQUIRE(devices.size() == 1);
	CHECK_FALSE(devices[0].State.has_value());
}

TEST_CASE("ReadDevices raises a failure to enumerate devices")
{
	const ComPtr<FakeEnumerator> enumerator = Make<FakeEnumerator>();
	enumerator->EnumerateResult = E_OUTOFMEMORY;

	CHECK_THROWS_AS(ReadDevices(*enumerator.Get(), DeviceFilter()), std::system_error);
}

TEST_CASE("ReadDevices releases every endpoint it was given")
{
	const ComPtr<FakeEnumerator> enumerator = Make<FakeEnumerator>();
	const ComPtr<FakeEndpoint> speakers = enumerator->Add({ .Id = L"speakers", .Flow = eRender });
	enumerator->Defaults = { { { eRender, eConsole }, speakers } };
	const ULONG before = ReferenceCount(*speakers.Get());

	ReadDevices(*enumerator.Get(), DeviceFilter());

	CHECK(ReferenceCount(*speakers.Get()) == before);
}
