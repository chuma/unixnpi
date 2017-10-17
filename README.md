# UnixNPI
## a Newton OS package installer for POSIX-compatible systems

UnixNPI is a command-line program which will install Newton OS package files to
a Newton OS device connected via a serial port, using the native *Dock* or
*Connection* tool on the Newton OS device.

This eliminates the need to use classic MacOS or Windows software to install
Newton OS packages from a PC, and allows any POSIX-compatible system to install
Newton OS packages.

With the help of additional packages installed on the Newton OS device, UnixNPI
can take advantage of faster serial port speeds, up to 230,400bps.

## Requirements

* a POSIX-compatible system with a C compiler, standard C library and headers,
and (GNU?) `make`
* a serial port
* a Newton OS device connected with a proper serial cable

## Compilation

`make` will build a binary named `unixnpi`.

See the `INSTALL` file for more details.

## Command arguments

`unixnpi [-s speed] [-d device] PkgFiles...`

`[speed]` is in bits per second, and defaults to 38400 (the default connection
speed of the *Serial* connection profile in Newton OS)

`[device]` is the path to the serial port device, and defaults to `/dev/newton`.

All other arguments are assumed to be names of Newton OS package files to send
to the device.

## Usage

1. Execute `unixnpi` with the proper arguments.  An informational banner will be
output, followed by the message `Waiting to connect`
2. On the Newton OS device, open the built-in connection tool from the **Extras** drawer:
  1. For the MessagePad 100, 110, and 120, and 130 (Newton OS 2.0 or less):
    1. Open **Connection**.
    2. Choose either **Serial** (for OS 2.0) or **DOS or Windows PC** (for OS 1.x) and tap the **Connect** button.
  2. For the MessagePad 2000, 2100, and eMate 300:
    1. Open **Dock**.
    2. Choose **Serial** and tap the **Connect** button.
3. The connection should be established and data transferred to the Newton.

## Authors

Originally written by Richard C.I. Li

Maintained by Victor Rehorst <victor@chuma.org>

Previous maintainer: Chayim I. Kirshen <ckirshen@linuxppc.org>

patches by:
* Hendrik Lipka <hendrik.lipka@gmx.de>
* Phil <phil@squack.com>

## History

The original UnixNPI homepage (for historical purposes ONLY) was at http://www.ee.cityu.edu.hk/~clli/unixnpi.html and is archived at https://web.archive.org/web/20121030074828/http://www.ee.cityu.edu.hk:80/~clli/unixnpi.html

The original open-source UnixNPI homepage is at http://unixnpi.sf.net

The latest UnixNPI homepage is http://github.com/chuma/unixnpi
