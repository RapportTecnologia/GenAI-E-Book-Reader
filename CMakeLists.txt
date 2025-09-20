cmake_minimum_required(VERSION 3.16)

project(GenAI_EBook_Reader VERSION 0.1.7 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Sources
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/ai/*.cpp"
    "src/ui/*.cpp"
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

target_include_directories(genai_reader PRIVATE include ${CMAKE_SOURCE_DIR}/src ${Qt6_INCLUDE_DIRS})

target_link_libraries(genai_reader PRIVATE Qt6::Widgets Qt6::PdfWidgets Qt6::Network)
target_link_libraries(genai_reader PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(genai_reader PRIVATE cmark)
target_compile_definitions(genai_reader PRIVATE HAVE_QT_WEBENGINE)

# Define to enable Qt-specific code paths unconditionally
target_compile_definitions(genai_reader PRIVATE USE_QT HAVE_QT_PDF HAVE_QT_NETWORK)

# Enable AUTOMOC/AUTORCC/AUTOUIC for convenience
set_target_properties(genai_reader PROPERTIES
  AUTOMOC ON
  AUTORCC ON
  AUTOUIC ON
)

# Add cmark for Markdown to HTML conversion
include(FetchContent)
FetchContent_Declare(
  cmark
  GIT_REPOSITORY https://github.com/commonmark/cmark.git
  GIT_TAG 0.30.2 # Using a stable version
)
FetchContent_MakeAvailable(cmark)

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
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/
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
find_program(LINUXDEPLOY_BIN linuxdeploy)
find_program(CONVERT_BIN convert)
find_program(QMAKE6_BIN qmake6)

if(UNIX AND NOT APPLE)
  set(TOOLS_CACHE_DIR "${CMAKE_SOURCE_DIR}/.cache/tools")
  file(MAKE_DIRECTORY ${TOOLS_CACHE_DIR})

  set(LINUXDEPLOY_PATH "${TOOLS_CACHE_DIR}/linuxdeploy-x86_64.AppImage")
  set(LINUXDEPLOY_QT_PLUGIN_PATH "${TOOLS_CACHE_DIR}/linuxdeploy-plugin-qt-x86_64.AppImage")

  if(NOT EXISTS "${LINUXDEPLOY_PATH}")
    message(STATUS "Downloading linuxdeploy...")
    file(DOWNLOAD
      https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
      "${LINUXDEPLOY_PATH}"
      SHOW_PROGRESS
    )
    execute_process(COMMAND chmod +x "${LINUXDEPLOY_PATH}")
  endif()

  if(NOT EXISTS "${LINUXDEPLOY_QT_PLUGIN_PATH}")
    message(STATUS "Downloading linuxdeploy-plugin-qt...")
    file(DOWNLOAD
      https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
      "${LINUXDEPLOY_QT_PLUGIN_PATH}"
      SHOW_PROGRESS
    )
    execute_process(COMMAND chmod +x "${LINUXDEPLOY_QT_PLUGIN_PATH}")
  endif()

  set(LINUXDEPLOY_BIN "${LINUXDEPLOY_PATH}")

  add_custom_target(dist
    DEPENDS genai_reader
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/bin"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/share/applications"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:genai_reader>" "${APPDIR}/usr/bin/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/resources/appimage/genai-reader.desktop" "${APPDIR}/usr/share/applications/"

    COMMAND ${CMAKE_COMMAND} -E env QMAKE="${QMAKE6_BIN}" "${LINUXDEPLOY_BIN}" --appdir "${APPDIR}" -e "${APPDIR}/usr/bin/genai_reader" -d "${APPDIR}/usr/share/applications/genai-reader.desktop" -i "${APPDIR}/usr/share/icons/hicolor/256x256/apps/genai-reader.png" --output appimage --plugin qt 
    COMMAND ${CMAKE_COMMAND} -E rename "${CMAKE_BINARY_DIR}/GenAI_E-Book_Reader-x86_64.AppImage" "${DIST_DIR}/GenAI_EBook_Reader-v${PROJECT_VERSION}-x86_64.AppImage"

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Creating AppImage bundle in ${DIST_DIR}"
  )
endif()

# Custom target to create icons. This will be a dependency for the main executable.
add_custom_target(create_icons
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/share/icons/hicolor/128x128/apps"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/share/icons/hicolor/256x256/apps"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/share/icons/hicolor/512x512/apps"
    COMMAND ${CONVERT_BIN} "${CMAKE_SOURCE_DIR}/docs/imgs/logo-do-projeto.png" -resize 128x128! "${APPDIR}/usr/share/icons/hicolor/128x128/apps/genai-reader.png"
    COMMAND ${CONVERT_BIN} "${CMAKE_SOURCE_DIR}/docs/imgs/logo-do-projeto.png" -resize 256x256! "${APPDIR}/usr/share/icons/hicolor/256x256/apps/genai-reader.png"
    COMMAND ${CONVERT_BIN} "${CMAKE_SOURCE_DIR}/docs/imgs/logo-do-projeto.png" -resize 512x512! "${APPDIR}/usr/share/icons/hicolor/512x512/apps/genai-reader.png"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Creating application icons"
)

# Make the main target depend on icon creation
add_dependencies(genai_reader create_icons)

# Install rule for desktop integration
install(FILES resources/appimage/genai-reader.desktop DESTINATION share/applications)
install(DIRECTORY "${APPDIR}/usr/share/icons/hicolor/" DESTINATION share/icons)

# ----------------------------------------------------------------------------
# GitHub Release Target
# Creates a git tag for the current version and pushes it to origin.
# Usage:
#   cmake --build . --target github_release
# ----------------------------------------------------------------------------
find_program(GH_BIN gh)

if(NOT GH_BIN)
  message(WARNING "GitHub CLI 'gh' not found. The 'github_release' target will not be able to create releases on GitHub.")
endif()

add_custom_target(github_release
    DEPENDS docs dist
    COMMAND git add -A
    COMMAND git commit -m "Release version ${PROJECT_VERSION}"
    COMMAND git push
    COMMAND git tag -a "v${PROJECT_VERSION}" -m "Release version ${PROJECT_VERSION}"
    COMMAND git push origin "v${PROJECT_VERSION}"
#    COMMAND ${GH_BIN} release create "v${PROJECT_VERSION}" --title "Release v${PROJECT_VERSION}" --notes "See CHANGELOG.md for details."
#    COMMAND ${GH_BIN} release upload "v${PROJECT_VERSION}" "${DIST_DIR}/GenAI_EBook_Reader-v${PROJECT_VERSION}-x86_64.AppImage"
    COMMAND ${GH_BIN} release upload "v${PROJECT_VERSION}" "${CMAKE_SOURCE_DIR}/APPIMAGE.md"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Creating GitHub release and uploading artifacts for v${PROJECT_VERSION}"
    VERBATIM
)



