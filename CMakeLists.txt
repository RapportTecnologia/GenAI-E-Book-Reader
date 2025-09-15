cmake_minimum_required(VERSION 3.16)

project(GenAI_EBook_Reader VERSION 0.1.5 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Sources
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

# Require Qt6 with Widgets, PdfWidgets and Network; WebEngineWidgets is optional
find_package(Qt6 REQUIRED COMPONENTS Widgets PdfWidgets Network)
find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets QUIET)
if (Qt6WebEngineWidgets_FOUND)
  message(STATUS "Qt6 WebEngineWidgets found: enabling rich chat rendering")
else()
  message(STATUS "Qt6 WebEngineWidgets NOT found: building without rich chat (Markdown/MathJax disabled)")
endif()

add_executable(genai_reader ${APP_SOURCES})

target_include_directories(genai_reader PRIVATE include ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(genai_reader PRIVATE Qt6::Widgets Qt6::PdfWidgets Qt6::Network)
target_link_libraries(genai_reader PRIVATE Qt6::WebEngineWidgets)
target_compile_definitions(genai_reader PRIVATE HAVE_QT_WEBENGINE)

# Define to enable Qt-specific code paths unconditionally
target_compile_definitions(genai_reader PRIVATE USE_QT HAVE_QT_PDF HAVE_QT_NETWORK)

# Enable AUTOMOC/AUTORCC/AUTOUIC for convenience
set_target_properties(genai_reader PROPERTIES
  AUTOMOC ON
  AUTORCC ON
  AUTOUIC ON
)

# Add application resources (logo, etc.)
target_sources(genai_reader PRIVATE ${CMAKE_SOURCE_DIR}/resources/app.qrc)

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

# ----------------------------------------------------------------------------
# Distribution artifacts
# Creates into ${CMAKE_SOURCE_DIR}/dist:
#  - genai_reader-v${PROJECT_VERSION}-linux-x86_64
#  - genai_reader-v${PROJECT_VERSION}-linux-x86_64.tar.gz
#  - GenAI_EBook_Reader-${PROJECT_VERSION}-x86_64.AppImage (if appimagetool is found)

set(DIST_DIR "${CMAKE_SOURCE_DIR}/dist")
set(DIST_BASENAME "genai_reader-v${PROJECT_VERSION}-linux-x86_64")
set(APPDIR "${CMAKE_BINARY_DIR}/AppDir")
find_program(APPIMAGETOOL appimagetool)

add_custom_target(dist
  DEPENDS genai_reader
  COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}"
  # Copy versioned binary
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:genai_reader>" "${DIST_DIR}/${DIST_BASENAME}"
  # Create tar.gz with the versioned binary inside
  COMMAND ${CMAKE_COMMAND} -E chdir "${DIST_DIR}" ${CMAKE_COMMAND} -E tar cfvz "${DIST_BASENAME}.tar.gz" "${DIST_BASENAME}"
  COMMENT "Creating distribution artifacts in ${DIST_DIR}"
)

