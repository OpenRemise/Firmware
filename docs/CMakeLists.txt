find_package(Doxygen QUIET COMPONENTS dot mscgen)

if(Doxygen_FOUND)
  # Get more recent version of Doxygen (remove once updated in Ubuntu 24.04)
  cpmaddpackage(
    NAME
    Doxygen
    URL
    "https://github.com/doxygen/doxygen/releases/download/Release_1_12_0/doxygen-1.12.0.linux.bin.tar.gz"
    VERSION
    1.12.0
    DOWNLOAD_ONLY
    ON)
  set(DOXYGEN_EXECUTABLE ${Doxygen_SOURCE_DIR}/bin/doxygen)

  # Get PlantUML
  cpmaddpackage(
    NAME
    PlantUML
    URL
    "https://github.com/plantuml/plantuml/releases/download/v1.2024.7/plantuml-1.2024.7.jar"
    VERSION
    1.2024.7
    DOWNLOAD_ONLY
    ON
    DOWNLOAD_NO_EXTRACT
    ON)
  set(PLANTUML_JAR_PATH ${PlantUML_SOURCE_DIR}/plantuml-1.2024.7.jar)

  # Get Doxygen Awesome
  cpmaddpackage("gh:jothepro/doxygen-awesome-css@2.3.3")

  # Get logo and fonts
  file(DOWNLOAD
       "https://github.com/OpenRemise/.github/raw/master/data/images/logo.svg"
       ${CMAKE_SOURCE_DIR}/data/images/logo.svg)
  file(
    DOWNLOAD
    "https://github.com/OpenRemise/.github/raw/master/data/fonts/GlacialIndifference-Bold.otf"
    ${CMAKE_CURRENT_BINARY_DIR}/html/GlacialIndifference-Bold.otf)
  file(
    DOWNLOAD
    "https://github.com/OpenRemise/.github/raw/master/data/fonts/GlacialIndifference-Regular.otf"
    ${CMAKE_CURRENT_BINARY_DIR}/html/GlacialIndifference-Regular.otf)

  # Get KiCanvas (https://kicanvas.org/) and schematics/boards
  file(DOWNLOAD "https://kicanvas.org/kicanvas/kicanvas.js"
       ${CMAKE_CURRENT_BINARY_DIR}/html/kicanvas.js)
  file(
    DOWNLOAD
    "https://github.com/OpenRemise/ESP32S3Board/raw/master/esp32s3board.kicad_sch"
    ${CMAKE_CURRENT_BINARY_DIR}/html/esp32s3board.kicad_sch)
  file(
    DOWNLOAD
    "https://github.com/OpenRemise/ESP32S3Board/raw/master/esp32s3board.kicad_pcb"
    ${CMAKE_CURRENT_BINARY_DIR}/html/esp32s3board.kicad_pcb)
  file(
    DOWNLOAD
    "https://github.com/OpenRemise/DRV8328Shield/raw/master/drv8323shield.kicad_sch"
    ${CMAKE_CURRENT_BINARY_DIR}/html/drv8323shield.kicad_sch)
  file(
    DOWNLOAD
    "https://github.com/OpenRemise/DRV8328Shield/raw/master/drv8323shield.kicad_pcb"
    ${CMAKE_CURRENT_BINARY_DIR}/html/drv8323shield.kicad_pcb)

  set(DOXYGEN_PROJECT_NUMBER ${PROJECT_VER})
  set(DOXYGEN_PROJECT_LOGO ${CMAKE_SOURCE_DIR}/data/images/logo.svg)
  set(DOXYGEN_REPEAT_BRIEF NO)
  set(DOXYGEN_MULTILINE_CPP_IS_BRIEF YES)
  set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
  set(DOXYGEN_NUM_PROC_THREADS 0)
  set(DOXYGEN_EXTRACT_ALL YES)
  set(DOXYGEN_EXTRACT_PRIVATE YES)
  set(DOXYGEN_EXTRACT_STATIC YES)
  set(DOXYGEN_EXTRACT_LOCAL_CLASSES YES)
  set(DOXYGEN_EXTRACT_LOCAL_METHODS YES)
  set(DOXYGEN_EXTRACT_ANON_NSPACES YES)
  set(DOXYGEN_IMAGE_PATH ${CMAKE_SOURCE_DIR}/data)
  set(DOXYGEN_HTML_HEADER ${CMAKE_CURRENT_LIST_DIR}/header.html)
  set(DOXYGEN_HTML_FOOTER ${CMAKE_CURRENT_LIST_DIR}/footer.html)
  set(DOXYGEN_HTML_EXTRA_STYLESHEET
      ${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome.css
      ${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-sidebar-only.css
      ${CMAKE_CURRENT_LIST_DIR}/custom.css)
  set(DOXYGEN_HTML_EXTRA_FILES
      ${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-fragment-copy-button.js
      ${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-interactive-toc.js
      ${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-tabs.js)
  set(DOXYGEN_HTML_COLORSTYLE LIGHT)
  set(DOXYGEN_HTML_COLORSTYLE_HUE 209)
  set(DOXYGEN_HTML_COLORSTYLE_SAT 255)
  set(DOXYGEN_HTML_COLORSTYLE_GAMMA 113)
  set(DOXYGEN_GENERATE_TREEVIEW YES)
  set(DOXYGEN_MACRO_EXPANSION YES)
  set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
  set(DOXYGEN_UML_LOOK YES)
  set(DOXYGEN_DOT_IMAGE_FORMAT svg)
  set(DOXYGEN_PLANTUML_JAR_PATH ${PLANTUML_JAR_PATH})

  doxygen_add_docs(FirmwareDocs ${CMAKE_SOURCE_DIR}/src
                   ${CMAKE_SOURCE_DIR}/CHANGELOG.md)
endif()
