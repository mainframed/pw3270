
## GTK Based 3270 terminal emulator

<img src="https://raw.githubusercontent.com/PerryWerneck/PerryWerneck/master/screenshots/mvs-tk4.png" alt="Screenshot">

pw3270 is a modern, GTK-based, completely free tn3270 emulator. 

Created originally for Banco do Brasil, it's now an official Brazilian Government Public Software project, and is used worldwide. 

See more details at https://softwarepublico.gov.br/social/pw3270/

<!-- https://github.com/igrigorik/ga-beacon -->

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
![CodeQL](https://github.com/PerryWerneck/libv3270/workflows/CodeQL/badge.svg?branch=master)

## Installation

[<img src="https://cdn.jsdelivr.net/npm/simple-icons@3.9.0/icons/opensuse.svg" alt="OpenSUSE Linux" height="80px">](https://software.opensuse.org/package/pw3270)
[<img src="https://cdn.jsdelivr.net/npm/simple-icons@3.9.0/icons/linux.svg" alt="Linux distribution" height="80px">](https://software.opensuse.org/download.html?project=home%3APerryWerneck%3Apw3270&package=pw3270)

### Linux (latest and unstable versions)

For the supported distributions get the install repositories and instructions from https://software.opensuse.org/download.html?project=home%3APerryWerneck%3Apw3270&package=pw3270

### Windows

[<img src="https://cdn.jsdelivr.net/npm/simple-icons@3.9.0/icons/dropbox.svg" alt="Get from dropbox" height="80px">](https://www.dropbox.com/sh/2qy3s6b5s4o4bws/AAAubHE8SBG7r6CJSKPflKN0a?dl=0)
[<img src="https://cdn.jsdelivr.net/npm/simple-icons@3.9.0/icons/googledrive.svg" alt="Get from google drive" height="80px">](https://drive.google.com/drive/folders/1tmtKzGujLVvnIV_knWQXl_TBEC3_9ucL?usp=sharing)
[<img src="https://cdn.jsdelivr.net/npm/simple-icons@3.9.0/icons/microsoftonedrive.svg" alt="Get from Microsoft One Drive" height="80px">](https://onedrive.live.com/?id=D8B46DA0372A6F1A%212208&cid=D8B46DA0372A6F1A)

## Building for Linux

	```shell
	$ git clone https://github.com/PerryWerneck/pw3270.git
	$ cd pw3270
	$ ./autogen.sh
	$ make all
	$ sudo make install
	```

## Building for Windows

### Cross-compiling on SuSE Linux (Native or WSL)

1. Add the MinGW Repositories

	```shell
	$ sudo zypper ar obs://windows:mingw:win32 mingw32
	$ sudo zypper ar obs://windows:mingw:win64 mingw64
	$ sudo zypper ar obs://home:PerryWerneck:pw3270 pw3270
	$ sudo zypper ref
	```

2. Run the build script

	```shell
	$ wget https://github.com/PerryWerneck/pw3270/blob/master/win/pack.sh
	$ ./pack.sh --pre-reqs
	```

### Windows native with MSYS2

1. Build and install [libv3270](../../../libv3270)

2. Install required packages

	```shell
	$ pacman -S mingw-w64-x86_64-imagemagick mingw-w64-x86_64-optipng mingw-w64-x86_64-inkscape
	```

2. Get pw3270 sources from git

	```
	$ git clone https://github.com/PerryWerneck/pw3270.git ./pw3270
	```

3. Build library using the mingw shell

	```
	$ cd pw3270
	$ ./autogen.sh
	$ make all
	```


## Building for macOS (using homebrew)

1. Build and install [libv3270](../../../libv3270)

2. Install additional dependencies

	```shell
	$ brew install adwaita-icon-theme imagemagick
	```

3. Configure and build

	```shell
	$ ./autogen.sh
	$ make all
	````

4. Create app bundle

	```shell
	$ cd macos
	$ ./bundle
	````

