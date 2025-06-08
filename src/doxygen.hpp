// Copyright (C) 2025 Vincent Hamp
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

/// Documentation
///
/// \file   doxygen.hpp
/// \author Vincent Hamp
/// \date   26/02/2023

/// \details
/// Welcome to the OpenRemise [Firmware](https://github.com/OpenRemise/Firmware)
/// documentation, the software powering all
/// [OpenRemise boards](\ref page_hw_reference).
///
/// \note
/// This documentation is intended for developers. General information on usage
/// can be found on the main page at [openremise.at](https://openremise.at).
///
// clang-format off
/// \mainpage Introduction
/// | Getting Started                                                                                                                                                                                            | API Reference                                                                                                                                                                                              | HW Reference                                                                                                                    |
/// | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
/// | [![](stopwatch.svg)](page_getting_started.html)                                                                                                                                                            | [![](api.svg)](page_api_reference.html)                                                                                                                                                                    | [![](pcb.svg)](page_hw_reference.html)                                                                                          |
/// | <div style="max-width:200px">New to the codebase? Check out the \ref page_getting_started guides. Set up a development environment and learn about the firmwares architecture and it's key concepts.</div> | <div style="max-width:200px">The \ref page_api_reference contains a detailed description of the inner workings of the firmwares individual modules. It assumes an understanding of the key concepts.</div> | <div style="max-width:200px">Browse schematics and layouts of all supported boards in the \ref page_hw_reference section.</div> |
///
/// <div class="section_buttons">
/// | Next                      |
/// | ------------------------: |
/// | \ref page_getting_started |
/// </div>
// clang-format on

/// \page page_getting_started Getting Started
/// \details
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
/// | Previous   | Next                  |
/// | :--------- | --------------------: |
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
/// host](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-guides/host-apps.html).
///
/// We recommend either an [Arch](https://archlinux.org) (e.g.
/// [Garuda](https://garudalinux.org) or [Manjaro](https://manjaro.org)) or
/// [Ubuntu](https://ubuntu.com) based distribution, so all of the following
/// steps refer to those.
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
/// - [CMake](https://cmake.org) ( >= 3.25 )
/// - [GCC](https://gcc.gnu.org) ( >= 13.2.0 )
/// - [ESP-IDF](https://github.com/espressif/esp-idf) ( >= 5.3 )
/// - [Ninja](https://ninja-build.org) ( >= 1.10.2 )
/// - Optional
///   - for building documentation
///     - [Doxygen](https://www.doxygen.nl/index.html) ( >= 1.12.0 )
///     - [Graphviz](https://graphviz.org) ( >= 12.1.1 )
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
/// instructions](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/get-started/linux-macos-setup.html#get-started-get-esp-idf)
/// and clone the ESP-IDF repository into a directory called `esp` in our home
/// directory.
///
/// [GitHub](https://github.com) allows you to clone a repository either over
/// SSH or HTTPS. If you have the option, we recommend using SSH as we believe
/// it [simplifies commit
/// signing](https://docs.github.com/en/authentication/managing-commit-signature-verification/about-commit-signature-verification#ssh-commit-signature-verification).
/// <div class="tabbed">
/// - <b class="tab-title">SSH</b>
///   ```sh
///   mkdir -p ~/esp
///   cd ~/esp
///   git clone -b \idf_ver --recursive git@github.com:higaski/esp-idf.git
///   ```
/// - <b class="tab-title">HTTPS</b>
///   ```sh
///   mkdir -p ~/esp
///   cd ~/esp
///   git clone -b \idf_ver --recursive https://github.com/higaski/esp-idf.git
///   ```
/// </div>
///
/// Attentive readers will immediately notice that we have explicitly checked
/// out version
/// [\idf_ver](https://github.com/espressif/esp-idf/releases/tag/\idf_ver) here.
/// We advise against using a different version than the one currently used by
/// the [master branch](https://github.com/OpenRemise/Firmware/tree/master).
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
/// \subsection subsection_development_flutter Flutter (optional)
/// It is optionally possible to build the
/// [Frontend](https://github.com/OpenRemise/Frontend) from source while
/// compiling the firmware. However, this requires the installation of
/// [Flutter](https://flutter.dev/). For more information please visit the
/// [Development](https://openremise.at/Frontend/page_development.html) section
/// of the frontend documentation.
///
/// \subsection subsection_development_vscode VSCode (optional)
/// We generally recommend [VSCode](https://code.visualstudio.com) for
/// development, but this step remains entirely optional. Feel free to otherwise
/// fire up your favorite editor or IDE.
/// <div class="tabbed">
/// - <b class="tab-title">Arch</b>
///   ```sh
///   sudo pamac install visual-studio-code-bin
///   ```
/// - <b class="tab-title">Ubuntu 24.04</b>
///   ```sh
///   snap install code --classic
///   ```
/// </div>
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
/// \warning
/// Please make sure that you do not just **execute** the script! The script
/// must be **sourced**, the leading dot and the space are important.
/// Furthermore, the variables that this script creates are only valid for the
/// current terminal session. Each time a new session is opened, the script must
/// be re-run.
///
/// If everything has worked up to this point, Espressif will, after a series of
/// debug outputs, reward you with the following message.
///
/// ```sh
/// Done! You can now compile ESP-IDF projects.
/// ```
///
/// We'll try this out right away by starting one of three CMake build
/// configurations. These configurations differ in the compiler's optimization
/// level (Debug corresponds to `-Og`, Release corresponds to `-Os`), and above
/// all in whether the USB peripheral is initialized as a [built-in JTAG
/// interface](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-guides/jtag-debugging/configure-builtin-jtag.html).
/// <div class="tabbed">
/// - <b class="tab-title">Debug</b>
///   ```sh
///   cmake --preset "Debug"
///   ```
/// - <b class="tab-title">Debug JTAG</b>
///   ```sh
///   cmake --preset "Debug JTAG"
///   ```
/// - <b class="tab-title">Release</b>
///   ```sh
///   cmake --preset "Release"
///   ```
/// </div>
///
/// \note
/// In principle, the `sdkconfig` files can be stacked in any way, e.g. to
/// create a Release JTAG build. For obvious reasons though, a firmware compiled
/// with the built-in JTAG interface can no longer be used for other USB
/// connections. See \ref page_config for more details.
///
/// After the CMake configure stage has been successful, we just need to run the
/// actual build stage.
/// ```sh
/// cmake --build build --parallel
/// ```
///
/// The terminal output of our build tool tells us if the binary has been built
/// successfully.
/// ```sh
/// Creating esp32s3 image...
/// Merged 2 ELF sections
/// Successfully created esp32s3 image.
/// Generated OpenRemise/Firmware/build/Firmware.bin
/// ```
///
/// \section section_development_flash Flash
/// In order to flash the generated binary (or actually, binaries) onto a board,
/// we need to put the board into the bootloader. This is done by
/// - Connecting the board to the USB-C port
/// - Setting the BOOT jumper
/// - Switching on the power supply
///
/// Afterwards we can use the `flash` command of the
/// [idf.py](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-guides/tools/idf-py.html)
/// frontend to upload the firmware.
/// ```sh
/// idf.py flash
/// ```
///
/// \warning
/// Be careful, this command actually flashes 5 binaries at once and not just
/// the application. This includes things like the
/// [bootloader](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-guides/bootloader.html),
/// the [partition
/// table](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-guides/partition-tables.html)
/// and the [NVS
/// storage](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-reference/storage/nvs_flash.html).
/// Again, you can find further details in \ref page_config. If you already have
/// a board with a running firmware and just want to flash the application, you
/// can do this with the following command.
/// ```sh
/// idf.py app-flash
/// ```
///
/// \section section_development_debug Debug
/// The idf.py frontend we just saw is only one of two ways to communicate with
/// our ESP chip over USB. The other way is the built-in JTAG interface.
/// Espressif's own documentation has a diagram that illustrates the two
/// possibilities well. The convenient part is that both connections are made
/// via the same USB cable. This is made possible by the ESP32-S3 chip itself,
/// which provides two USB channels, one for JTAG and the other for the USB
/// terminal connection we have already used.
///
/// \image html jtag-debugging-overview.jpg "JTAG debugging"
///
/// Before we can establish a debugging connection with GDB though, we need to
/// grant the appropriate rights to communicate with the OpenOCD device. To do
/// this, we download the [udev
/// rules](https://github.com/espressif/openocd-esp32/raw/master/contrib/60-openocd.rules)
/// from Espressif's [OpenOCD
/// repository](https://github.com/espressif/openocd-esp32) to
/// `/etc/udev/rules.d`.
// clang-format off
/// \page page_development Development
/// \details \tableofcontents
/// ```sh
/// sudo wget https://github.com/espressif/openocd-esp32/raw/master/contrib/60-openocd.rules -P /etc/udev/rules.d
/// ```
// clang-format on
/// \page page_development Development
/// \details \tableofcontents
///
/// With the udev rules installed, we can now start a debug session. We
/// recommend using VSCode for this, but of course you can also do it manually
/// via CLI.
/// <div class="tabbed">
/// - <b class="tab-title">VSCode</b>
///   To debug the firmware directly in VSCode, we install the GDB debugger
///   extension [Native
///   Debug](https://marketplace.visualstudio.com/items?itemName=webfreak.debug)
///   from the Visual Studio Marketplace. If you are not familiar with VSCode
///   extensions, you can take a look at the [official
///   documentation](https://code.visualstudio.com/docs/editor/extension-marketplace).
///
///   Running the debugger is done by
///   - Connecting the board to the USB-C port
///   - Switching on the power supply
///   - Hitting F5
/// - <b class="tab-title">CLI</b>
///   To debug the firmware directly via the CLI we start 2x terminal sessions.
///   In the first session we launch OpenOCD with the built-in JTAG interface
///   configuration.
///   ```sh
///   cd ~/.espressif/tools/openocd-esp32/v0.12.0-esp32-20240318/openocd-esp32
///   bin/openocd -f board/esp32s3-builtin.cfg
///   ```
///   And in the second session, started in our project folder, we connect to
///   GDB.
///   ```sh
///   xtensa-esp32-elf-gdb -x gdbinit build/Firmware.elf
///   ```
/// </div>
///
/// \section section_development_test Test
/// A very useful feature of the ESP-IDF framework is the ability to run
/// applications on the
/// [host](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-guides/host-apps.html).
/// This allows us to write [unit
/// tests](https://en.wikipedia.org/wiki/Unit_testing) that do not need to be
/// run directly on a board, but can be run on the host (and in the CI
/// pipeline). The tests themselves are written with
/// [GoogleTest](https://github.com/google/googletest).
///
/// To build the tests executable we run the `Tests` build configuration.
/// ```sh
/// cmake --preset "Tests"
/// cmake --build build --parallel
/// ```
///
/// And directly execute the [.elf
/// file](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format).
/// ```sh
/// ╰─λ ./build/Firmware.elf
/// I (21886608) port: Starting scheduler.
/// [==========] Running 18 tests from 5 test suites.
/// [----------] Global test environment set-up.
/// [----------] 4 tests from DccTest
/// [ RUN      ] DccTest.loco_to_base_to_json
/// [       OK ] DccTest.loco_to_base_to_json (0 ms)
/// ...
/// [----------] Global test environment tear-down
/// [==========] 18 tests from 5 test suites ran. (100 ms total)
/// [  PASSED  ] 18 tests.
/// ```
///
/// \section section_development_doc Doc
/// If Doxygen was found during CMake's configuration phase, the `FirmwareDocs`
/// target can be built to create the documentation.
/// ```sh
/// cmake --build build --target FirmwareDocs
/// ```
///
/// <div class="section_buttons">
/// | Previous                  | Next             |
/// | :------------------------ | ---------------: |
/// | \ref page_getting_started | \ref page_config |
/// </div>

/// \page page_config Configuration
/// \details \tableofcontents
/// The configuration of the firmware takes place in two phases. First of all,
/// the Kconfig language (known from the Linux kernel) is used to define
/// hundreds of different compile-time constants for the ESP-IDF framework. And
/// then project-specific constants and globals are defined in a header file.
///
/// \section section_config_kconfig Kconfig
/// With the help of
/// [Kconfig](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-reference/kconfig.html),
/// hundreds of different definitions are managed within the ESP-IDF framework.
/// These definitions range from rather unimportant (e.g. type of newlib line
/// ending) to absolutely essential (e.g. CPU frequency). As output, Kconfig
/// creates a file called `sdkconfig`, which contains all the definitions
/// necessary for the build. However, this file is not tracked by Git, only a
/// file called
/// [`sdkconfig.defaults`](https://github.com/OpenRemise/Firmware/raw/master/sdkconfig.defaults)
/// ends up in the repository, which contains the deviations from the default
/// settings.
///
/// \section section_config_config_hpp config.hpp
/// The project-specific definitions are made in a header called `config.hpp`,
/// which gets **included in all translation units by default**. It contains,
/// among other things, pin assignments, constants such as the ADC sampling
/// rate, general global variables, FreeRTOS task handler and stack size, etc.
/// etc.
///
/// An example of the FreeRTOS definitions would be the struct for the ADC task.
/// ```cpp
/// inline struct AdcTask {
///   static constexpr auto name{"analog::adc"};
///   static constexpr auto stack_size{4096uz};
///   static constexpr UBaseType_t priority{ESP_TASK_PRIO_MAX - 2u};
///   static constexpr auto timeout{200u};
///   TaskHandle_t handle{};
/// } adc_task;
/// ```
///
/// \section section_config_partition_table Partition Table
/// The firmware has a [partition
/// table](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-guides/partition-tables.html)
/// that allows us to divide the entire available flash memory into partitions
/// for different uses. Specifically, this division is done through a CSV file
/// called
/// [`partitions.csv`](https://github.com/OpenRemise/Firmware/raw/master/partitions.csv).
/// The following table shows the distribution.
///
// clang-format off
/// \page page_config Configuration
/// \details \tableofcontents
/// | Name    | Type | Subtype  | Size   | Description                                                          |
/// | ------- | ---- | -------- | ------ | -------------------------------------------------------------------- |
/// | otadata | data | ota      | 8K     | Bootloader uses this data to know which ota partition to execute     |
/// | ota_0   | app  | ota_0    | 6M     | First application partition (toggles to second on successful update) |
/// | ota_1   | app  | ota_1    | 6M     | Second application partition (toggles to first on successful update) |
/// | nvs     | data | nvs      | 6M     | Stores settings, locomotives and accessories                         |
/// | data    | data | littlefs | 14272K | Stores additional data (e.g. images)                                 |
// clang-format on
/// \page page_config Configuration
/// \details \tableofcontents
///
/// \warning
/// The partition table cannot be divided completely freely but is subject to
/// certain
/// [restrictions](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-reference/peripherals/spi_flash/spi_flash_optional_feature.html#restrictions).
///
/// \subsection subsection_config_ota OTA
/// The `ota_0` and `ota_1` entries are the actual app partitions, i.e. the
/// complete firmware **including the frontend** is stored here. The
/// abbreviation
/// [OTA](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-reference/system/ota.html)
/// refers to the so-called <b>O</b>ver <b>T</b>he <b>A</b>ir update capability
/// of the ESP32-S3. The update mechanism of the ESP-IDF framework toggles the
/// active partition after each successful update. This also ensures that an
/// error during the update does not lead to a bricked device.
///
/// \subsection subsection_config_nvs NVS
/// The `nvs` partition uses key-value pairs to store device settings,
/// locomotives and accessories. The default settings are contained in a CVS
/// file called
/// [`nvs.csv`](https://github.com/OpenRemise/Firmware/raw/master/nvs.csv) and
/// are included when flashing a firmware.
///
/// \subsection subsection_config_data Data
/// The data partition is currently reserved but unused. It is planned to use a
/// [LittleFS](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-guides/file-system-considerations.html#littlefs-fs-section)
/// file system for this partition.
///
/// \section section_config_performance Performance
/// To ensure that the firmware runs smoothly, there are a few important points
/// to consider regarding performance.
///
/// \subsection subsection_config_performance_cpu CPU
/// With the dual core architecture of the ESP32-S3, it is **very important** to
/// pay close attention to which task is running on which core. To simplify
/// this, Espressif provides a FreeRTOS extension in the form of the
/// [`xTaskCreatePinnedToCore`](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-reference/system/freertos_additions.html?highlight=xtaskcreatepinnedtocore#_CPPv423xTaskCreatePinnedToCore14TaskFunction_tPCKcK8uint32_tPCv11UBaseType_tPC12TaskHandle_tK10BaseType_t)
/// function, which allows to explicitly specify the core to which the task
/// should get pinned to.
///
/// One pitfall that can easily occur here is not considering the initialization
/// of the peripherals. Although Espressif offers its own core-considering API
/// when creating tasks, this is completely missing for all peripherals. This
/// means that while initializing a peripheral, tasks and interrupts may be
/// allocated in the background, which either have their own Kconfig option
/// (e.g.
/// [LWIP_TCPIP_TASK_AFFINITY_CPU0](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-reference/kconfig.html#config-lwip-tcpip-task-affinity)),
/// or are simply allocated on the core that called the function. To work around
/// this, there is the utility function `invoke_on_core()`, which executes a
/// passed function on an assigned core.
///
/// \note
/// `invoke_on_core()` works in blocking mode and is really only used during
/// startup.
///
/// The following table provides an overview of the distribution of tasks across
/// cores.
///
/// | Task                     | Core |
/// | ------------------------ | ---- |
/// | analog::adc_task         | 1    |
/// | analog::temp_task        | 1    |
/// | dcc::task                | 1    |
/// | decup::task              | 1    |
/// | mdu::task                | 1    |
/// | ota::task                | 1    |
/// | out::track::dcc::task    | 1    |
/// | out::track::decup::task  | 1    |
/// | out::track::mdu::task    | 1    |
/// | out::zusi::task          | 1    |
/// | usb::rx_task             | 1    |
/// | usb::tx_task             | 1    |
/// | usb::ulf_dcc_ein::task   | 1    |
/// | usb::ulf_decup_ein::task | 1    |
/// | usb::ulf_susiv2::task    | 1    |
/// | wifi::task               | 0    |
/// | z21::task                | 0    |
/// | zusi::task               | 1    |
///
/// All other ESP-IDF internal tasks ([ESP
/// Timer](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-reference/system/esp_timer.html),
/// FreeRTOS Timer, [Event
/// Loop](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-reference/system/esp_event.html),
/// [lwIP](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-guides/lwip.html)
/// and the [Wi-Fi
/// Driver](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-guides/wifi.html))
/// are pinned to core 0 either by default or by Kconfig options.
///
/// \note
/// Espressif itself refers to core 0 as `PRO_CPU` and core 1 as `APP_CPU`. The
/// official recommendation is to reserve core 0 for protocol-related processing
/// such as Wi-Fi or Bluetooth, while the rest of the application's tasks should
/// run on core 1.
///
/// \subsection subsection_config_performance_iram IRAM
/// By default, all code is executed from flash cache. This means that it is
/// possible for the CPU to have to wait on a "cache miss" while the next
/// instructions are loaded from flash. This is not an option for real-time
/// critical code. Fortunately, the ESP-IDF framework offers a solution for this
/// in the form of the IRAM (Instruction RAM) attribute. Functions that contain
/// the `IRAM_ATTR` macro in the signature are automatically placed in the IRAM
/// section.
///
/// In addition to individual application functions, ESP-IDF code from various
/// peripherals (or at least parts of it) must also be placed in IRAM. To do
/// this, the corresponding kConfig option, e.g.,
/// `CONFIG_GPIO_CTRL_FUNC_IN_IRAM`, must be enabled.
///
/// \subsection subsection_config_performance_wifi WiFi
/// Espressif has a dedicated article on [How to Improve Wi-Fi
/// Performance](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-guides/wifi.html#how-to-improve-wi-fi-performance),
/// which, in addition to the various recommended configurations, also contains
/// a lot of information about how the Wi-Fi/LwIP protocol stack actually works.
/// Unfortunately, most settings consume a lot of internal RAM, which is why the
/// firmware mainly stays at the defaults.
///
/// There is one exception, however. The firmware increases the size of all
/// `*RECVMBOX` settings. The number of these mailboxes largely determines how
/// many messages can be queued for various protocols. The default size is quite
/// small and causes burst transmissions of many small messages to result in
/// errors.
///
/// For further details regarding general performance I recommend the Espressif
/// article [Speed
/// Optimization](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-guides/performance/speed.html).
///
/// <div class="section_buttons">
/// | Previous              | Next                   |
/// | :-------------------- | ---------------------: |
/// | \ref page_development | \ref page_architecture |
/// </div>

/// \page page_architecture Architecture
/// \details \tableofcontents
/// The entire software stack is divided into three layers. At the top are the
/// interfaces, which represent the connections to the outside world. In the
/// middle, various services run as middlewares that take care of processing the
/// data and generate various output signals from it. And underneath that there
/// are drivers that use these signals to control the hardware outputs. A more
/// detailed version of the upcoming diagram can be found at the [bottom of the
/// page](#section_architecture_diagram) (\emoji :warning: not mobile friendly).
// clang-format off
/// \page page_architecture Architecture
/// \details \tableofcontents
/// \startuml "Architecture overview"
/// !theme mono
/// skinparam defaultFontName "Glacial Indifference"
///
/// database "Storage" {
/// }
///
/// frame "Interfaces" {
/// }
///
/// frame "Middlewares" {
/// }
///
/// frame "Drivers" {
/// }
///
/// Interfaces -d-> Middlewares
/// Middlewares -d-> Drivers
///
/// 'Links
/// url of Storage is [[page_architecture.html#section_architecture_storage]]
/// url of Interfaces is [[page_architecture.html#section_architecture_interfaces]]
/// url of Middlewares is [[page_architecture.html#section_architecture_middlewares]]
/// url of Drivers is [[page_architecture.html#section_architecture_drivers]]
/// \enduml
// clang-format on
/// \page page_architecture Architecture
/// \details \tableofcontents
///
/// \section section_architecture_interfaces Interfaces
/// The interface layer contains modules that are used to receive and transmit
/// data. They initialize various communication peripherals such as USB and WiFi
/// and manage the data transfer via them. The received data is processed and
/// then forwarded to the corresponding services in the middleware layer.
///
/// \section section_architecture_middlewares Middlewares
/// The modules in the middleware layer contain services that connect the
/// interfaces with the outputs in the driver layer. An example of this is the
/// mdu::Service, which can accept a
/// [WebSocket](https://en.wikipedia.org/wiki/WebSocket) connection from the
/// ::http::sta::Server and then processes commands for an update of the decoder
/// software. The service then starts the corresponding \ref
/// out::track::mdu::task_function "MDU task" in the driver layer, which then
/// generates the necessary signals on the track. The communication is
/// bidirectional, received feedback is sent back to the server via service. In
/// more complex situations, middleware services may also communicate directly
/// with each other. An example of this is the communication between the
/// dcc::Service and the z21::Service.
///
/// \section section_architecture_drivers Drivers
/// In the driver layer there are modules that contain some form of IO. For the
/// most part, these are modules that generate signals controlled by middleware
/// services. Other modules initialize various hardware peripherals such as the
/// ADC or WiFi.
///
/// \section section_architecture_storage Storage
/// Although these modules do not necessarily represent a layer, they are still
/// a separate part of the architecture and manage the different flash memory
/// partitions. Currently this only involves the
/// [nvs](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-reference/storage/nvs_flash.html)
/// module which stores settings or locomotives in a key-value pair system. All
/// modules are considered global and can be accessed from all other layers.
///
/// \section section_architecture_diagram Diagram
// clang-format off
/// \page page_architecture Architecture
/// \details \tableofcontents
/// \startuml "Architecture diagram"
/// !theme mono
/// skinparam defaultFontName "Glacial Indifference"
///
/// database "Storage" {
///   package "mem" as storage_mem {
///     package "nvs" as storage_mem_nvs {
///       [Accessories]
///       [Locos]
///       [Settings]
///       Accessories -[hidden] Locos
///       Locos -[hidden] Settings
///     }
///   }
/// }
///
/// frame "Interfaces" {
///   package "http" as intf_http {
///     package "ap" as intf_http_ap {
///       interface "HTTP" as http_ap
///       [Server] as intf_http_ap_server
///       http_ap - intf_http_ap_server
///     }
///     package "sta" as intf_http_sta {
///       interface "HTTP" as http_sta
///       [Server] as intf_http_sta_server
///       http_sta - intf_http_sta_server
///     }
///   }
///   package "mDNS" as intf_mdns {
///     interface mDNS
///   }
///   package "udp" as intf_udp {
///     interface UDP
///   }
///   package "usb" as intf_usb {
///     [Tasks] as intf_usb_tasks
///     USB - intf_usb_tasks
///   }
/// }
///
/// frame "Middlewares" {
///   package "dcc" as mw_dcc {
///     [Service] as mw_dcc_service
///   }
///   package "decup" as mw_decup {
///     [Service] as mw_decup_service
///   }
///   package "mdu" as mw_mdu {
///     [Service] as mw_mdu_service
///   }
///   package "ota" as mw_ota {
///     [Service] as mw_ota_service
///   }
///   package "ulf" as mw_ulf {
///     [ULF_DCC_EIN]
///     [ULF_DECUP_EIN]
///     [ULF_SUSIV2]
///   }
///   package "z21" as mw_z21 {
///     [Service] as mw_z21_service
///   }
///   package "zusi" as mw_zusi {
///     [Service] as mw_zusi_service
///   }
/// }
///
/// frame "Drivers" {
///   package "analog" as drv_analog {
///     [Task] as analog_task
///   }
///   package "led" as drv_led {
///     [" "]
///   }
///   package "out" as drv_out {
///     package "track" as drv_out_track {
///       package "dcc" as drv_out_track_dcc {
///         [Task] as drv_out_track_dcc_task
///       }
///       package "decup" as drv_out_track_decup {
///         [Task] as drv_out_track_decup_task
///       }
///       package "mdu" as drv_out_track_mdu {
///         [Task] as drv_out_track_mdu_task
///       }
///     }
///     package "zusi" as drv_out_zusi {
///       [Task] as drv_out_zusi_task
///     }
///   }
///   package "trace" as drv_trace {
///     ["  "]
///   }
///   package "wifi" as drv_wifi {
///     [Task] as drv_wifi_task
///   }
/// }
///
/// intf_http_sta_server <--> mw_dcc_service
/// intf_http_sta_server <--> mw_decup_service
/// intf_http_sta_server <--> mw_mdu_service
/// intf_http_sta_server <--> mw_ota_service
/// intf_http_sta_server <--> mw_z21_service
/// intf_http_sta_server <--> mw_zusi_service
///
/// intf_usb_tasks <--> ULF_DCC_EIN
/// intf_usb_tasks <--> ULF_DECUP_EIN
/// intf_usb_tasks <--> ULF_SUSIV2
///
/// mw_z21_service <-l-> mw_dcc_service
///
/// mw_dcc_service <--> drv_out_track_dcc_task
/// mw_decup_service <--> drv_out_track_decup_task
/// mw_mdu_service <--> drv_out_track_mdu_task
/// mw_zusi_service <--> drv_out_zusi_task
///
/// [ULF_DCC_EIN] <--> drv_out_track_dcc_task
/// [ULF_DECUP_EIN] <--> drv_out_track_decup_task
/// [ULF_SUSIV2] <--> drv_out_zusi_task
///
/// UDP <--> mw_z21_service
///
/// 'Links
/// url of storage_mem is [[page_mem.html]]
/// url of storage_mem_nvs is [[page_mem.html#section_mem_nvs]]
///
/// url of intf_http is [[page_http.html]]
/// url of intf_http_ap is [[page_http.html#section_http_ap]]
/// url of intf_http_sta is [[page_http.html#section_http_sta]]
/// url of intf_mdns is [[page_mdns.html]]
/// url of intf_udp is [[page_udp.html]]
/// url of intf_usb is [[page_usb.html]]
///
/// url of mw_dcc is [[page_dcc.html]]
/// url of mw_decup is [[page_decup.html]]
/// url of mw_mdu is [[page_mdu.html]]
/// url of mw_ota is [[page_ota.html]]
/// url of mw_ulf is [[page_usb.html]]
/// url of mw_zusi is [[page_zusi.html]]
/// url of mw_z21 is [[page_z21.html]]
///
/// url of drv_analog is [[page_analog.html]]
/// url of drv_led is [[page_led.html]]
/// url of drv_out is [[page_out.html]]
/// url of drv_out_track is [[page_out.html#section_out_track]]
/// url of drv_out_track_dcc is [[page_out.html#subsection_out_track_dcc]]
/// url of drv_out_track_decup is [[page_out.html#subsection_out_track_decup]]
/// url of drv_out_track_mdu is [[page_out.html#subsection_out_track_mdu]]
/// url of drv_out_zusi is [[page_out.html#section_out_zusi]]
/// url of drv_trace is [[page_trace.html]]
/// url of drv_wifi is [[page_wifi.html]]
/// \enduml
// clang-format on
/// \page page_architecture Architecture
/// \details \tableofcontents
///
/// \note
/// This diagram contains links. Clicking on a module takes you to the
/// corresponding \ref page_api_reference.
///
/// <div class="section_buttons">
/// | Previous         | Next                   |
/// | :--------------- | ---------------------: |
/// | \ref page_config | \ref page_control_flow |
/// </div>

/// \page page_control_flow Control Flow
/// \details \tableofcontents
/// \todo \ref state, how to change between different operating modes
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
// clang-format off
/// \page page_api_reference API Reference
/// \details
/// | Chapter                | Content                                                                                                                                                                                                                           |
/// | ---------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
/// | \subpage page_analog   | ADC measurements, overcurrent                                                                                                                                                                                                     |
/// | \subpage page_dcc      | Operation and service mode, [DCC](https://github.com/ZIMO-Elektronik/DCC) command generation, BiDi decoding                                                                                                                       |
/// | \subpage page_decup    | DECUP [ZPP](https://github.com/ZIMO-Elektronik/ZPP) and [ZSU](https://github.com/ZIMO-Elektronik/ZSU) updates (WebSocket service)                                                                                                 |
/// | \subpage page_led      | Dimming LEDs                                                                                                                                                                                                                      |
/// | \subpage page_http     | Access point (AP) and station (STA) HTTP servers                                                                                                                                                                                  |
/// | \subpage page_mdns     | mDNS services                                                                                                                                                                                                                     |
/// | \subpage page_mdu      | MDU [ZPP](https://github.com/ZIMO-Elektronik/ZPP) and [ZSU](https://github.com/ZIMO-Elektronik/ZSU) updates (WebSocket service)                                                                                                   |
/// | \subpage page_mem      | NVS memory for settings, locos, accessories                                                                                                                                                                                       |
/// | \subpage page_ota      | OTA update (WebSocket service)                                                                                                                                                                                                    |
/// | \subpage page_out      | Drivers, signal generation of [DCC](https://github.com/ZIMO-Elektronik/DCC), [DECUP](https://github.com/ZIMO-Elektronik/DECUP), [MDU](https://github.com/ZIMO-Elektronik/MDU) and [ZUSI](https://github.com/ZIMO-Elektronik/ZUSI) |
/// | \subpage page_trace    | Debug purposes                                                                                                                                                                                                                    |
/// | \subpage page_udp      | Create and bind UDP socket                                                                                                                                                                                                        |
/// | \subpage page_ulf      | [ULF_COM](https://github.com/ZIMO-Elektronik/ULF_COM) services (ZIMO specific serial protocols)                                                                                                                                   |
/// | \subpage page_usb      | Create [CDC](https://en.wikipedia.org/wiki/USB_communications_device_class) device                                                                                                                                                |
/// | \subpage page_wifi     | Setup WiFi in AP or STA mode                                                                                                                                                                                                      |
/// | \subpage page_z21      | [Z21](https://github.com/ZIMO-Elektronik/Z21) server (UDP and WebSocket service)                                                                                                                                                  |
/// | \subpage page_zusi     | ZUSI [ZPP](https://github.com/ZIMO-Elektronik/ZPP) updates (WebSocket service)                                                                                                                                                    |
// clang-format on
/// \page page_api_reference API Reference
/// \details
///
/// <div class="section_buttons">
/// | Previous               | Next             |
/// | :--------------------- | ---------------: |
/// | \ref page_control_flow | \ref page_analog |
/// </div>

/// \page page_hw_reference HW Reference
/// \details \tableofcontents
/// \htmlonly
/// <script type="module" src="kicanvas.js"></script>
/// \endhtmlonly
///
/// \section section_hw_reference_active Active
/// \subsection subsection_hw_reference_active_s3main S3Main
/// \htmlonly
/// <kicanvas-embed controls="full">
///   <kicanvas-source src="s3main.kicad_sch"></kicanvas-source>
///   <kicanvas-source src="s3main.kicad_pcb"></kicanvas-source>
/// </kicanvas-embed>
/// \endhtmlonly
///
/// \section section_hw_reference_deprecated Deprecated
/// \subsection subsection_hw_reference_deprecated_esp32s3board ESP32S3Board
/// \htmlonly
/// <kicanvas-embed controls="full">
///   <kicanvas-source src="esp32s3board.kicad_sch"></kicanvas-source>
///   <kicanvas-source src="esp32s3board.kicad_pcb"></kicanvas-source>
/// </kicanvas-embed>
/// \endhtmlonly
///
/// \subsection subsection_hw_reference__deprecated_drv8323shield DRV8323Shield
/// \htmlonly
/// <kicanvas-embed controls="full">
///   <kicanvas-source src="drv8323shield.kicad_sch"></kicanvas-source>
///   <kicanvas-source src="drv8323shield.kicad_pcb"></kicanvas-source>
/// </kicanvas-embed>
/// \endhtmlonly
///
/// <div class="section_buttons">
/// | Previous       |
/// | :------------- |
/// | \ref page_zusi |
/// </div>
