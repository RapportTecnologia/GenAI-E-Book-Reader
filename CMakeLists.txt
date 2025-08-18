cmake_minimum_required(VERSION 3.16)

project(GenAI_EBook_Reader VERSION 0.1.3 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_WITH_QT "Build UI with Qt (Widgets)" ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Sources
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

if(BUILD_WITH_QT)
  # Try Qt6 first, then Qt5
  find_package(Qt6 COMPONENTS Widgets PdfWidgets Network QUIET)
  if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets Network QUIET)
  endif()

  if(Qt6_FOUND OR Qt5_FOUND)
    message(STATUS "Building with Qt Widgets")

    if(Qt6_FOUND)
      set(QT_PREFIX Qt6)
      set(QT_LIB Qt6::Widgets)
      # Optional Pdf support on Qt6
      if(TARGET Qt6::PdfWidgets)
        set(QT_PDF Qt6::PdfWidgets)
      endif()
      if(TARGET Qt6::Network)
        set(QT_NET Qt6::Network)
      endif()
    else()
      set(QT_PREFIX Qt5)
      set(QT_LIB Qt5::Widgets)
      # Qt5 Pdf not configured in this project by default
      if(TARGET Qt5::Network)
        set(QT_NET Qt5::Network)
      endif()
    endif()

    add_executable(genai_reader ${APP_SOURCES})

    target_include_directories(genai_reader PRIVATE include ${CMAKE_SOURCE_DIR}/src)

    target_link_libraries(genai_reader PRIVATE ${QT_LIB})
    if(DEFINED QT_PDF)
      target_link_libraries(genai_reader PRIVATE ${QT_PDF})
      target_compile_definitions(genai_reader PRIVATE HAVE_QT_PDF)
    endif()
    if(DEFINED QT_NET)
      target_link_libraries(genai_reader PRIVATE ${QT_NET})
      target_compile_definitions(genai_reader PRIVATE HAVE_QT_NETWORK)
    endif()

    # Define to enable Qt-specific code paths
    target_compile_definitions(genai_reader PRIVATE USE_QT)

    # Enable AUTOMOC/AUTORCC/AUTOUIC for convenience
    set_target_properties(genai_reader PROPERTIES
      AUTOMOC ON
      AUTORCC ON
      AUTOUIC ON
    )

    # Add application resources (logo, etc.)
    target_sources(genai_reader PRIVATE ${CMAKE_SOURCE_DIR}/resources/app.qrc)

  else()
    message(WARNING "Qt not found. Building console placeholder. Set BUILD_WITH_QT=OFF to silence.")
  endif()
endif()

# Fallback: simple console app when Qt is unavailable
if(NOT TARGET genai_reader)
  add_executable(genai_reader ${APP_SOURCES})
  target_include_directories(genai_reader PRIVATE include ${CMAKE_SOURCE_DIR}/src)
endif()

# ----------------------------------------------------------------------------
# .env bootstrap (development convenience)
# If a .env file does not exist at the project root, create one with defaults.
# You can override defaults by passing -DPHPLIST_URL_DEFAULT=... etc. to CMake.
set(PHPLIST_URL_DEFAULT "http://arvoredossaberes.com.br/newsletter/lists/api/v2" CACHE STRING "Default PHPList API URL to write to .env if it doesn't exist")
set(PHPLIST_USER_DEFAULT "" CACHE STRING "Default PHPList user to write to .env if it doesn't exist")
set(PHPLIST_PASS_DEFAULT "" CACHE STRING "Default PHPList password to write to .env if it doesn't exist")

set(ENV_FILE_PATH "${CMAKE_SOURCE_DIR}/.env")
if(NOT EXISTS "${ENV_FILE_PATH}")
  file(WRITE "${ENV_FILE_PATH}" "PHPLIST_URL=${PHPLIST_URL_DEFAULT}\nPHPLIST_USER=${PHPLIST_USER_DEFAULT}\nPHPLIST_PASS=${PHPLIST_PASS_DEFAULT}\n")
  message(STATUS "Created default .env at ${ENV_FILE_PATH}")
else()
  message(STATUS ".env already exists at ${ENV_FILE_PATH}; leaving unchanged")
endif()

# Doxygen target
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
  set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile)
  add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
else()
  message(STATUS "Doxygen not found. 'docs' target will not be available.")
endif()
