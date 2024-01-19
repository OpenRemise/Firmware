find_package(Doxygen QUIET COMPONENTS dot mscgen)

if(Doxygen_FOUND)
  cpmaddpackage("gh:jothepro/doxygen-awesome-css@2.3.1")

  set(DOXYGEN_PROJECT_LOGO ${CMAKE_SOURCE_DIR}/data/images/head.svg)
  set(DOXYGEN_MULTILINE_CPP_IS_BRIEF YES)
  set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
  set(DOXYGEN_NUM_PROC_THREADS 0)
  set(DOXYGEN_EXTRACT_ALL YES)
  set(DOXYGEN_EXTRACT_PRIVATE YES)
  set(DOXYGEN_EXTRACT_STATIC YES)
  set(DOXYGEN_EXTRACT_LOCAL_CLASSES YES)
  set(DOXYGEN_EXTRACT_LOCAL_METHODS YES)
  set(DOXYGEN_EXTRACT_ANON_NSPACES YES)
  set(DOXYGEN_IMAGE_PATH ${CMAKE_SOURCE_DIR}/data/images)
  set(DOXYGEN_HTML_HEADER header.html)
  set(DOXYGEN_HTML_EXTRA_STYLESHEET
      ${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome.css
      ${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-sidebar-only.css
      custom.css)
  set(DOXYGEN_HTML_EXTRA_FILES
      ${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-fragment-copy-button.js
      ${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-interactive-toc.js)
  set(DOXYGEN_HTML_COLORSTYLE LIGHT)
  set(DOXYGEN_GENERATE_TREEVIEW YES)
  set(DOXYGEN_MACRO_EXPANSION YES)
  set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
  set(DOXYGEN_UML_LOOK YES)
  set(DOXYGEN_DOT_IMAGE_FORMAT svg)

  doxygen_add_docs(FirmwareDocs ${CMAKE_SOURCE_DIR}/src
                   ${CMAKE_SOURCE_DIR}/CHANGELOG.md ${CMAKE_SOURCE_DIR}/TODO.md)
endif()
