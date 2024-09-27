/// Documentation
///
/// \file   doxygen.hpp
/// \author Vincent Hamp
/// \date   26/02/2023

/// \details
/// Welcome to the OpenRemise [Firmware](https://github.com/OpenRemise/Firmware)
/// documentation, the software powering all
/// [Remise boards](\ref page_hw_reference).
///
/// \note
/// This documentation is intended for developers. General information on usage
/// can be found on the main page at [openremise.at](https://openremise.at/).
///
// clang-format off
/// \mainpage Introduction
/// | Getting Started                                                                                                                                                                        | API Reference                                                                                                                                                                           | HW Reference                                                                                                       |
/// | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------ |
/// | [![](icons/stopwatch.png)](page_getting_started.html)                                                                                                                                  | [![](icons/api.png)](page_api_reference.html)                                                                                                                                           | [![](icons/pcb.png)](page_hw_reference.html)                                                                       |
/// | New to the codebase?<br>Check out the \ref page_getting_started <br>guides. Setup a development<br>environment and learn about<br>the firmwares architecture<br>and it's key concepts. | The \ref page_api_reference contains a<br>detailed description of the inner<br>workings of the firmwares<br> individual modules.<br>It assumes an understanding of<br>the key concepts. | Browse schematics and layouts<br>of all supported boards in the<br>\ref page_hw_reference section.<br><br><br><br> |
///
/// <div class="section_buttons">
/// | Next                      |
/// | ------------------------: |
/// | \ref page_getting_started |
/// </div>
// clang-format on

/// \page page_getting_started Getting Started
/// The aim of this section is to help you set up a development environment and
/// provide you with a 30.000ft overview of the firmware and some of it's key
/// concepts.
///
/// After we have successfully set up a development environment, compiled the
/// firmware and flashed it onto a board, we will look at how the firmware is
/// configured, how the system architecture works and which phases and operating
/// modes the software can go through.
///
/// | Chapter                    | Content                         |
/// | -------------------------- | ------------------------------- |
/// | \subpage page_development  | Setup a development environment |
/// | \subpage page_config       | Configuring the boards          |
/// | \subpage page_architecture | System architecture             |
/// | \subpage page_control_flow | From bootloader to scheduler    |
///
/// <div class="section_buttons">
/// | Previous   | Next                   |
/// | :--------- | ---------------------: |
/// | \ref index | \ref page_development |
/// </div>

/// \page page_development Development
/// \details \tableofcontents
/// In this chapter, we set up a development environment that allows us to
/// create the firmware, its unit tests, and the documentation. For this we
/// install the <b>Esp</b>ressif <b>I</b>oT <b>D</b>evelopment <b>F</b>ramework
/// ([ESP-IDF](https://github.com/espressif/esp-idf)) which, in addition to the
/// obvious support for ESP chips, can also be compiled for Linux. This has the
/// advantage that our unit tests can run directly [on the
/// host](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/host-apps.html).
///
/// We recommend either an [Arch](https://archlinux.org/) or
/// [Ubuntu](https://ubuntu.com/) based distribution, so all of the following
/// steps refer to those two.
///
/// \section section_development_prerequisites Prerequisites
/// In order to start developing the firmware, we need to meet quite a few
/// prerequisites. Fortunately, most of them can be obtained directly from the
/// package manager. But before we do that, let's bring our system up to date.
/// <div class="tabbed">
/// - <b class="tab-title">Arch</b>
///   ```sh
///   sudo pacman -Syu --noconfirm
///   ```
/// - <b class="tab-title">Ubuntu 24.04</b>
///   ```sh
///   sudo apt update -y
///   sudo apt upgrade -y
///   ```
/// </div>
///
/// Without going into detail about each individual dependency, the most
/// important ones are CMake, a build system, GCC, a host compiler, Ninja, a
/// build tool, Doxygen, a documentation generator, and Graphviz, a graph
/// visualization software.
///
/// - [CMake](https://cmake.org/) ( >= 3.25 )
/// - [GCC](https://gcc.gnu.org/) ( >= 13.2.0 )
/// - [ESP-IDF](https://github.com/espressif/esp-idf) ( >= 5.3 )
/// - [Ninja](https://ninja-build.org/) ( >= 1.10.2 )
/// - Optional
///   - for building documentation
///     - [Doxygen](https://www.doxygen.nl/index.html) ( >= 1.12.0 )
///     - [Graphviz](https://graphviz.org/) ( >= 12.1.1 )
///
// clang-format off
/// \page page_development Development
/// \details \tableofcontents
/// <div class="tabbed">
/// - <b class="tab-title">Arch</b>
///   ```sh
///   sudo pacman -S --noconfirm bison ccache cmake dfu-util doxygen flex gcc git gperf graphviz jdk-openjdk libbsd libusb make ninja python ruby
///   ```
/// - <b class="tab-title">Ubuntu 24.04</b>
///   ```sh
///   sudo apt-get install -y bison build-essential ccache clang-format cmake dfu-util doxygen flex git gperf graphviz libbsd-dev libffi-dev libssl-dev libusb-1.0-0 ninja-build openjdk-21-jdk python3 python3-pip python3-venv ruby wget
///   ```
/// </div>
// clang-format on
/// \page page_development Development
/// \details \tableofcontents
///
/// \subsection subsection_development_esp_idf ESP-IDF
/// The ESP-IDF framework is the only dependency that we cannot get directly
/// from the packet manager. Instead, we follow [Espressif's
/// instructions](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/linux-macos-setup.html#get-started-get-esp-idf)
/// and clone the ESP-IDF repository into a directory called `esp` in our home
/// directory.
///
/// [GitHub](https://github.com/) allows you to clone a repository either over
/// SSH or HTTPS. If you have the option, we recommend using SSH as we believe
/// it [simplifies commit
/// signing]((https://docs.github.com/en/authentication/managing-commit-signature-verification/about-commit-signature-verification#ssh-commit-signature-verification)).
/// <div class="tabbed">
/// - <b class="tab-title">SSH</b>
///   ```sh
///   mkdir -p ~/esp
///   cd ~/esp
///   git clone -b v5.3 --recursive git@github.com:espressif/esp-idf.git
///   ```
/// - <b class="tab-title">HTTPS</b>
///   ```sh
///   mkdir -p ~/esp
///   cd ~/esp
///   git clone -b v5.3 --recursive https://github.com/espressif/esp-idf.git
///   ```
/// </div>
///
/// Attentive readers will immediately notice that we have explicitly checked
/// out version [5.3](https://github.com/espressif/esp-idf/releases/tag/v5.3)
/// here. We advise against using a different version than the one currently
/// used by the [master
/// branch](https://github.com/OpenRemise/Firmware/tree/master).
///
/// In addition to the ESP-IDF source code, the framework also requires some
/// tools such as the Xtensa compiler or various Python packages. To set these
/// up, Espressif provides an installation script that must be executed.
/// <div class="tabbed">
/// - <b class="tab-title">Fish</b>
///   ```sh
///   cd ~/esp/esp-idf
///   ./install.fish esp32s3
///   ```
/// - <b class="tab-title">Bash</b>
///   ```sh
///   cd ~/esp/esp-idf
///   ./install.sh esp32s3
///   ```
/// </div>
///
/// This script is available in different shell flavors. Please be careful to
/// run the correct one.
///
/// \note
/// If you're not sure which shell you're running you might execute [this cute
/// snippet](https://askubuntu.com/a/1022440) here.
/// ```sh
/// sh -c 'ps -p $$ -o ppid=' | xargs -I'{}' readlink -f '/proc/{}/exe'
/// ```
/// It prints the path to the executable of the shell from which it was
/// executed.
///
/// \section section_development_clone Clone
/// The firmware source code is also hosted on GitHub. As before, we can use
/// either SSH or HTTP to clone the
/// [repository](https://github.com/OpenRemise/Firmware). Using `git clone`
/// without any additional arguments will clone the latest version of the master
/// branch to the current working directory. After that, we can change into the
/// `Firmware` directory we've just created.
/// <div class="tabbed">
/// - <b class="tab-title">SSH</b>
///   ```sh
///   git clone git@github.com:OpenRemise/Firmware.git
///   cd Firmware
///   ```
/// - <b class="tab-title">HTTPS</b>
///   ```sh
///   git clone https://github.com/OpenRemise/Firmware.git
///   cd Firmware
///   ```
/// </div>
///
/// \section section_development_build Build
/// Before we can start building the actual firmware, we need to add the ESP-IDF
/// framework and its tools to the PATH environment variable. For this, there is
/// again a script available that needs to be **sourced**.
///
/// \warning
/// Please make sure that you do not just **execute** the script! The script
/// must be **sourced**, the leading dot and the space are important.
/// Furthermore, the variables that this script creates are only valid for the
/// current terminal session. Each time a new session is opened, the script must
/// be re-run.
/// <div class="tabbed">
/// - <b class="tab-title">Fish</b>
///   ```sh
///   . esp-idf.fish
///   ```
/// - <b class="tab-title">Bash</b>
///   ```sh
///   . esp-idf.sh
///   ```
/// </div>
///
/// If everything has worked up to this point, Espressif will, after a series of
/// debug outputs, reward you with the following message.
///
/// ```sh
/// Done! You can now compile ESP-IDF projects.
/// ```
///
/// We'll try this out right away by starting one of three build configurations.
/// These configurations differ in the compiler's optimization level (Debug
/// corresponds to `-Og`, Release corresponds to `-Os`), and above all in
/// whether the USB peripheral is initialized as a [built-in JTAG
/// interface](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/jtag-debugging/configure-builtin-jtag.html).
///
/// \note
/// In principle, the `sdkconfig` files can be stacked in any way, e.g. to
/// create a Release JTAG build.
///
/// <div class="tabbed">
/// - <b class="tab-title">Debug</b>
///   ```sh
///   cmake -Bbuild -GNinja \
///   -DCMAKE_TOOLCHAIN_FILE=$IDF_PATH/tools/cmake/toolchain-esp32s3.cmake \
///   -DPYTHON_DEPS_CHECKED=1 \
///   -DESP_PLATFORM=1 \
///   -DIDF_TARGET=esp32s3 \
///   -DCCACHE_ENABLE=0 \
///   -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.debug"
///   ```
/// - <b class="tab-title">Debug JTAG</b>
///   ```sh
///   cmake -Bbuild -GNinja \
///   -DCMAKE_TOOLCHAIN_FILE=$IDF_PATH/tools/cmake/toolchain-esp32s3.cmake \
///   -DPYTHON_DEPS_CHECKED=1 \
///   -DESP_PLATFORM=1 \
///   -DIDF_TARGET=esp32s3 \
///   -DCCACHE_ENABLE=0 \
///   -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.debug;sdkconfig.jtag"
///   ```
/// - <b class="tab-title">Release</b>
///   ```sh
///   cmake -Bbuild -GNinja \
///   -DCMAKE_TOOLCHAIN_FILE=$IDF_PATH/tools/cmake/toolchain-esp32s3.cmake \
///   -DPYTHON_DEPS_CHECKED=1 \
///   -DESP_PLATFORM=1 \
///   -DIDF_TARGET=esp32s3 \
///   -DCCACHE_ENABLE=0 \
///   -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.release"
///   ```
/// </div>
///
/// \warning
/// For obvious reasons, a firmware compiled with the built-in JTAG interface
/// can no longer be used for other USB connections.
///
/// \todo ninja!
///
/// \section section_development_flash Flash
/// \todo section_development_flash
///
/// \section section_development_test Test
/// \todo section_development_test
///
/// \section section_development_debug Debug
/// \todo section_development_debug
///
/// https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/DEBUGGING.md#using-nativedebug
/// udev rules! ->
/// https://github.com/espressif/openocd-esp32/blob/master/contrib/60-openocd.rules
///
/// <div class="section_buttons">
/// | Previous                  | Next             |
/// | :------------------------ | ---------------: |
/// | \ref page_getting_started | \ref page_config |
/// </div>

/// \page page_config Configuration
/// \todo write config page
///
/// <div class="section_buttons">
/// | Previous              | Next                    |
/// | :-------------------- | ----------------------: |
/// | \ref page_development | \ref page_architecture |
/// </div>

/// \page page_architecture Architecture
/// \details \tableofcontents
/// \todo document arch page
/// The entire software stack is divided into three layers.
///
/// \startuml
/// !theme mono
/// skinparam defaultFontName "Glacial Indifference"
/// scale max 1920*1080
///
/// database "Storage" {
///   package "mem" {
///     package "nvs" {
///       [Accessories]
///       [Locos]
///       [Settings]
///       Accessories -[hidden] Locos
///       Locos -[hidden] Settings
///     }
///     package "spiffs" {
///       [Partition]
///     }
///   }
/// }
///
/// frame "Interfaces" {
///   package "http" {
///     package "ap" {
///       interface "HTTP" as ap_http
///       [Server] as ap_server
///       ap_http - ap_server
///     }
///     package "sta" {
///       interface "HTTP" as sta_http
///       [Server] as sta_server
///       sta_http - sta_server
///     }
///   }
///   package "udp" {
///     interface UDP
///   }
///   package "usb" as interfaces_usb {
///     [Tasks] as usb_tasks
///     USB - usb_tasks
///   }
/// }
///
/// frame "Middlewares" {
///   package "dcc" {
///     [Service] as dcc_service
///   }
///   package "decup" {
///     [Service] as decup_service
///   }
///   package "mdu" {
///     [Service] as mdu_service
///   }
///   package "ota" {
///     [Service] as ota_service
///   }
///   package "usb" as middlewares_usb {
///     [DCC_EIN]
///     [DECUP_EIN]
///     [SUSIV2]
///   }
///   package "z21" {
///     [Service] as z21_service
///   }
///   package "zusi" as middlewares_zusi {
///     [Service] as zusi_service
///   }
/// }
///
/// frame "Drivers" {
///   package "analog" {
///     [Task] as analog_task
///   }
///   package "out" {
///     package "track" {
///       package "dcc" as out_track_dcc {
///         [Task] as out_track_dcc_task
///       }
///       package "decup" as out_track_decup {
///         [Task] as out_track_decup_task
///       }
///       package "mdu" as out_track_mdu {
///         [Task] as out_track_mdu_task
///       }
///     }
///     package "zusi" as drivers_zusi {
///       [Task] as out_zusi_task
///     }
///   }
/// }
///
/// '
/// sta_server <--> dcc_service
/// sta_server <--> decup_service
/// sta_server <--> mdu_service
/// sta_server <--> ota_service
/// sta_server <--> z21_service
/// sta_server <--> zusi_service
///
/// usb_tasks <--> DCC_EIN
/// usb_tasks <--> DECUP_EIN
/// usb_tasks <--> SUSIV2
///
/// dcc_service <--> out_track_dcc_task
/// decup_service <--> out_track_decup_task
/// mdu_service <--> out_track_mdu_task
/// zusi_service <--> out_zusi_task
///
/// [DCC_EIN] <--> out_track_dcc_task
/// [DECUP_EIN] <--> out_track_decup_task
/// [SUSIV2] <--> out_zusi_task
///
/// UDP <--> z21_service
///
/// 'Links
/// url of analog is [[page_analog.html]]
/// url of mem is [[page_mem.html]]
/// url of mdu is [[page_mdu.html]]
/// url of http is [[page_http.html]]
/// url of ota is [[page_ota.html]]
/// url of udp is [[page_udp.html]]
/// url of dcc is [[page_dcc.html]]
/// url of decup is [[page_decup.html]]
/// url of out is [[page_out.html]]
/// url of middlewares_zusi is [[page_zusi.html]]
/// url of interfaces_usb is [[page_usb.html]]
/// url of middlewares_usb is [[page_usb.html]]
/// url of z21 is [[page_z21.html]]
/// \enduml
///
/// \section section_architecture_interfaces Interfaces
/// Etiam quis est sed nibh imperdiet convallis nec tincidunt magna. Nunc
/// efficitur euismod justo. Nunc a turpis sed leo pharetra imperdiet nec vel
/// magna. Nunc in quam est. Vestibulum in laoreet eros. Vestibulum ac
/// condimentum felis. Ut vel cursus elit. Nullam scelerisque eros ante, sed
/// mattis dolor hendrerit sit amet. Donec ornare tempor mi ac egestas. Cras
/// volutpat nulla sem, volutpat gravida ligula elementum et. Donec justo ex,
/// tempor dignissim efficitur id, tincidunt rutrum lorem. Cras venenatis non
/// tortor vel elementum. Aliquam dapibus tristique est congue fringilla.
/// Integer mollis nunc at dignissim lacinia. Ut euismod lorem mauris. Morbi
/// iaculis est imperdiet faucibus interdum.
///
/// \section section_architecture_middlewares Middlewares
/// Etiam quis est sed nibh imperdiet convallis nec tincidunt magna. Nunc
/// efficitur euismod justo. Nunc a turpis sed leo pharetra imperdiet nec vel
/// magna. Nunc in quam est. Vestibulum in laoreet eros. Vestibulum ac
/// condimentum felis. Ut vel cursus elit. Nullam scelerisque eros ante, sed
/// mattis dolor hendrerit sit amet. Donec ornare tempor mi ac egestas. Cras
/// volutpat nulla sem, volutpat gravida ligula elementum et. Donec justo ex,
/// tempor dignissim efficitur id, tincidunt rutrum lorem. Cras venenatis non
/// tortor vel elementum. Aliquam dapibus tristique est congue fringilla.
/// Integer mollis nunc at dignissim lacinia. Ut euismod lorem mauris. Morbi
/// iaculis est imperdiet faucibus interdum.
///
/// \section section_architecture_drivers Drivers
/// Etiam quis est sed nibh imperdiet convallis nec tincidunt magna. Nunc
/// efficitur euismod justo. Nunc a turpis sed leo pharetra imperdiet nec vel
/// magna. Nunc in quam est. Vestibulum in laoreet eros. Vestibulum ac
/// condimentum felis. Ut vel cursus elit. Nullam scelerisque eros ante, sed
/// mattis dolor hendrerit sit amet. Donec ornare tempor mi ac egestas. Cras
/// volutpat nulla sem, volutpat gravida ligula elementum et. Donec justo ex,
/// tempor dignissim efficitur id, tincidunt rutrum lorem. Cras venenatis non
/// tortor vel elementum. Aliquam dapibus tristique est congue fringilla.
/// Integer mollis nunc at dignissim lacinia. Ut euismod lorem mauris. Morbi
/// iaculis est imperdiet faucibus interdum.
///
/// \section section_architecture_storage Storage
/// Etiam quis est sed nibh imperdiet convallis nec tincidunt magna. Nunc
/// efficitur euismod justo. Nunc a turpis sed leo pharetra imperdiet nec vel
/// magna. Nunc in quam est. Vestibulum in laoreet eros. Vestibulum ac
/// condimentum felis. Ut vel cursus elit. Nullam scelerisque eros ante, sed
/// mattis dolor hendrerit sit amet. Donec ornare tempor mi ac egestas. Cras
/// volutpat nulla sem, volutpat gravida ligula elementum et. Donec justo ex,
/// tempor dignissim efficitur id, tincidunt rutrum lorem. Cras venenatis non
/// tortor vel elementum. Aliquam dapibus tristique est congue fringilla.
/// Integer mollis nunc at dignissim lacinia. Ut euismod lorem mauris. Morbi
/// iaculis est imperdiet faucibus interdum.
///
/// <div class="section_buttons">
/// | Previous         | Next                   |
/// | :--------------- | ---------------------: |
/// | \ref page_config | \ref page_control_flow |
/// </div>

/// \page page_control_flow Control flow
/// \tableofcontents
/// \todo write control flow page
/// \section A
/// Some text in this section
///
/// \tableofcontents
/// \section B
/// More text in this one
///
/// <div class="section_buttons">
/// | Previous               | Next             |
/// | :--------------------- | ---------------: |
/// | \ref page_architecture | \ref page_api_reference |
/// </div>

/// \page page_api_reference API Reference
/// \details
/// The entire codebase is divided into several modules. Modules consist of a
/// set of source files containing a bunch of components
/// ([classes](https://en.cppreference.com/w/cpp/language/classes)) which are
/// logically related. Consequently, these classes are also in a separate
/// [namespace](https://en.cppreference.com/w/cpp/language/namespace) and
/// directory according to the module name. This follows the [physical
/// design](https://youtu.be/d3zMfMC8l5U) philosophy. Components from different
/// modules **never** directly depend on each other but only on
/// [interfaces](https://en.cppreference.com/book/intro/abstract_classes) or
/// [concepts](https://en.cppreference.com/w/cpp/language/constraints).
///
/// \note
/// The module definition is similar to the one from
/// [C++20](https://en.cppreference.com/w/cpp/language/modules).
///
/// | Chapter                | Content                                         |
/// | ---------------------- | ----------------------------------------------- |
/// | \subpage page_analog   | Analog                                          |
/// | \subpage page_dcc      | DCC                                             |
/// | \subpage page_http     | HTTP                                            |
/// | \subpage page_mdu      | MDU                                             |
/// | \subpage page_mem      | SPIFFS and NVS memory                           |
/// | \subpage page_ota      | OTA                                             |
/// | \subpage page_out      | Blub                                            |
/// | \subpage page_udp      | UDP                                             |
/// | \subpage page_usb      | USB                                             |
/// | \subpage page_wifi     | WiFi                                            |
/// | \subpage page_z21      | Z21                                             |
/// | \subpage page_zusi     | ZUSI                                            |

/// \page page_hw_reference HW Reference
/// \htmlonly
/// <script type="module" src="kicanvas.js"></script>
/// <kicanvas-embed controls="full">
///   <kicanvas-source src="esp32s3board.kicad_sch"></kicanvas-source>
///   <kicanvas-source src="esp32s3board.kicad_pcb"></kicanvas-source>
/// </kicanvas-embed>
/// \endhtmlonly
