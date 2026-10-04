# sd

Lists Windows audio devices, one per line, for a script to read. The built program is `bin/sd.exe`.

## Usage

```
Usage: sd [-f FILTER...] [-id]

Lists audio devices, one per line:
  [ID,]NAME,DESCRIPTION,FLOW,STATE[,ROLES]

NAME is the audio adapter and DESCRIPTION is the device on it. FLOW is
Render or Capture. ROLES are the roles the device is the default device
for (Console, Multimedia, Communications), joined with "|".

A field that contains a comma or a quote is quoted as in CSV. Control
and text-direction characters in a field are replaced with spaces, so
every line is exactly one device.

Options:
  -f FILTER...  List only matching devices. State filters: active,
                disabled, notpresent, unplugged. Flow filters: capture,
                render. Without a state filter every state is listed;
                without a flow filter both flows are.
  -id           Start each line with the device ID.
  -h, --help    Show this help.
```

Render devices are listed before capture devices, each group ordered by ID. Output is UTF-8 when piped or redirected.

The exit code is 0 on success, 1 when Windows reports a failure, and 2 for a command line that cannot be understood. Errors go to stderr.

## Building

Needs Visual Studio 2022 (or its Build Tools) with the C++ workload, and CMake 3.25 or newer. Configuring downloads the test framework, doctest. From the repository root:

```
cmake -S src/sd -B src/sd/build -G "Visual Studio 17 2022" -A x64
cmake --build src/sd/build --config Release
ctest --test-dir src/sd/build -C Release
cmake --install src/sd/build --config Release --prefix .
```

The last command writes `bin/sd.exe`. The C runtime is linked statically, so the program needs no Visual C++ redistributable.

## Layout

- `Options` parses the command line.
- `Endpoints` reads devices from Windows. It is the only code that touches COM.
- `Device` holds a device as plain data, and orders and formats it.
- `Output` writes the lines.
- `audiopyle.cpp` ties them together and turns failures into exit codes.
- `tests/` covers each of those. `FakeEndpoints.h` provides in-memory stand-ins for the Windows endpoint objects, so reading can be tested for cases a real machine rarely offers, such as a device with no name or a flow with no default device.
