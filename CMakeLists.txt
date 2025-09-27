cmake_minimum_required(VERSION 3.16)

# Extract version from CHANGELOG.md (first heading like: ## [X.Y.Z])
file(READ "${CMAKE_CURRENT_LIST_DIR}/CHANGELOG.md" _CHANGELOG_TXT)
string(REGEX MATCH "## \\[[0-9]+\\.[0-9]+\\.[0-9]+\\]" _CHANGELOG_MATCH "${_CHANGELOG_TXT}")
if(_CHANGELOG_MATCH)
  string(REGEX REPLACE ".*\\[([0-9]+\\.[0-9]+\\.[0-9]+)\\].*" "\\1" GENAI_VERSION "${_CHANGELOG_MATCH}")
  message(STATUS "Building GenAI_EBook_Reader version ${GENAI_VERSION}")
else()
  set(GENAI_VERSION "0.0.0")
  message(WARNING "Could not parse version from CHANGELOG.md; defaulting to ${GENAI_VERSION}")
endif()

# Re-run CMake automatically when CHANGELOG.md changes so version propagates into build
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/CHANGELOG.md")

message(STATUS "Building GenAI_EBook_Reader version ${GENAI_VERSION}")

project(GenAI_EBook_Reader VERSION ${GENAI_VERSION} LANGUAGES C CXX)

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
find_package(Qt6 REQUIRED COMPONENTS Widgets PdfWidgets Network PrintSupport)
find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets QUIET)
if (Qt6WebEngineWidgets_FOUND)
  message(STATUS "Qt6 WebEngineWidgets found: enabling rich chat rendering")
else()
  message(STATUS "Qt6 WebEngineWidgets NOT found: building without rich chat (Markdown/MathJax disabled)")
endif()

add_executable(genai_reader ${APP_SOURCES})

target_include_directories(genai_reader PRIVATE include ${CMAKE_SOURCE_DIR}/src ${Qt6_INCLUDE_DIRS})

target_link_libraries(genai_reader PRIVATE Qt6::Widgets Qt6::PdfWidgets Qt6::Network Qt6::PrintSupport)
target_link_libraries(genai_reader PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(genai_reader PRIVATE cmark)
target_compile_definitions(genai_reader PRIVATE HAVE_QT_WEBENGINE)

# Define to enable Qt-specific code paths unconditionally
target_compile_definitions(genai_reader PRIVATE USE_QT HAVE_QT_PDF HAVE_QT_NETWORK)

# 0.1.10: Documentation/UI marker for Function Calling capability indicator
# This macro does not change behavior; it allows conditional display text in the
# LLM settings dialog to reflect availability of the read-only indicator.
target_compile_definitions(genai_reader PRIVATE GENAI_HAS_FUNCTION_CALLING_INDICATOR=1)

# Inject application version into code (used by include/app/App.h)
# This keeps AppInfo::Version synchronized with CMake's PROJECT_VERSION
target_compile_definitions(genai_reader PRIVATE GENAI_APP_VERSION=\"${PROJECT_VERSION}\")

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
set(OPENROUTER_API_KEY_DEFAULT "" CACHE STRING "Default OpenRouter API key to write to .env if it doesn't exist")
set(GENERATIVA_API_KEY_DEFAULT "" CACHE STRING "Default GenerAtiva API key to write to .env if it doesn't exist")
set(PERPLEXITY_API_KEY_DEFAULT "" CACHE STRING "Default Perplexity API key to write to .env if it doesn't exist")

set(ENV_FILE_PATH "${CMAKE_SOURCE_DIR}/.env")
if(NOT EXISTS "${ENV_FILE_PATH}")
  file(WRITE "${ENV_FILE_PATH}" "OPENROUTER_API_KEY=${OPENROUTER_API_KEY_DEFAULT}\nGENERATIVA_API_KEY=${GENERATIVA_API_KEY_DEFAULT}\nPERPLEXITY_API_KEY=${PERPLEXITY_API_KEY_DEFAULT}\n")
  message(STATUS "Created default .env at ${ENV_FILE_PATH}")
else()
  # Ensure OPENROUTER_API_KEY key exists in .env; if missing, append it with default empty value
  file(READ "${ENV_FILE_PATH}" _ENV_EXISTING)
  string(FIND "${_ENV_EXISTING}" "OPENROUTER_API_KEY=" _HAS_OR_KEY)
  if(_HAS_OR_KEY EQUAL -1)
    file(APPEND "${ENV_FILE_PATH}" "OPENROUTER_API_KEY=${OPENROUTER_API_KEY_DEFAULT}\n")
    message(STATUS "Appended OPENROUTER_API_KEY to existing .env")
  endif()
  # Ensure GENERATIVA_API_KEY key exists in .env; if missing, append it with default empty value
  string(FIND "${_ENV_EXISTING}" "GENERATIVA_API_KEY=" _HAS_GEN_KEY)
  if(_HAS_GEN_KEY EQUAL -1)
    file(APPEND "${ENV_FILE_PATH}" "GENERATIVA_API_KEY=${GENERATIVA_API_KEY_DEFAULT}\n")
    message(STATUS "Appended GENERATIVA_API_KEY to existing .env")
  endif()
  # Ensure PERPLEXITY_API_KEY key exists in .env; if missing, append it with default empty value
  string(FIND "${_ENV_EXISTING}" "PERPLEXITY_API_KEY=" _HAS_PPLX_KEY)
  if(_HAS_PPLX_KEY EQUAL -1)
    file(APPEND "${ENV_FILE_PATH}" "PERPLEXITY_API_KEY=${PERPLEXITY_API_KEY_DEFAULT}\n")
    message(STATUS "Appended PERPLEXITY_API_KEY to existing .env")
  endif()
  # Ensure GOOGLE_CLIENT_ID and GOOGLE_CLIENT_SECRET exist in .env; append if missing
  string(FIND "${_ENV_EXISTING}" "GOOGLE_CLIENT_ID=" _HAS_GOOGLE_ID)
  if(_HAS_GOOGLE_ID EQUAL -1)
    file(APPEND "${ENV_FILE_PATH}" "GOOGLE_CLIENT_ID=\n")
    message(STATUS "Appended GOOGLE_CLIENT_ID to existing .env")
  endif()
  string(FIND "${_ENV_EXISTING}" "GOOGLE_CLIENT_SECRET=" _HAS_GOOGLE_SECRET)
  if(_HAS_GOOGLE_SECRET EQUAL -1)
    file(APPEND "${ENV_FILE_PATH}" "GOOGLE_CLIENT_SECRET=\n")
    message(STATUS "Appended GOOGLE_CLIENT_SECRET to existing .env")
  endif()
  message(STATUS ".env already exists at ${ENV_FILE_PATH}")
endif()

# Read OPENROUTER_API_KEY value from .env to inject into code
file(READ "${ENV_FILE_PATH}" _ENV_TXT)
set(OPENROUTER_API_KEY_VALUE "")
string(REGEX MATCH "OPENROUTER_API_KEY[ \t]*=([^\n\r]*)" _OR_MATCH "${_ENV_TXT}")
if(_OR_MATCH)
  # CMAKE_MATCH_1 holds the captured value
  set(OPENROUTER_API_KEY_VALUE "${CMAKE_MATCH_1}")
endif()
# Sanitize potential quotes or CRLF
string(REGEX REPLACE "^\"(.*)\"$" "\\1" OPENROUTER_API_KEY "${OPENROUTER_API_KEY_VALUE}")
string(REPLACE "\r" "" OPENROUTER_API_KEY "${OPENROUTER_API_KEY}")

# Read GENERATIVA_API_KEY value from .env to inject into code
set(GENERATIVA_API_KEY_VALUE "")
string(REGEX MATCH "GENERATIVA_API_KEY[ \t]*=([^\n\r]*)" _GEN_MATCH "${_ENV_TXT}")
if(_GEN_MATCH)
  set(GENERATIVA_API_KEY_VALUE "${CMAKE_MATCH_1}")
endif()
string(REGEX REPLACE "^\"(.*)\"$" "\\1" GENERATIVA_API_KEY "${GENERATIVA_API_KEY_VALUE}")
string(REPLACE "\r" "" GENERATIVA_API_KEY "${GENERATIVA_API_KEY}")

# Read PERPLEXITY_API_KEY value from .env to inject into code
set(PERPLEXITY_API_KEY_VALUE "")
string(REGEX MATCH "PERPLEXITY_API_KEY[ \t]*=([^\n\r]*)" _PPLX_MATCH "${_ENV_TXT}")
if(_PPLX_MATCH)
  set(PERPLEXITY_API_KEY_VALUE "${CMAKE_MATCH_1}")
endif()
string(REGEX REPLACE "^\"(.*)\"$" "\\1" PERPLEXITY_API_KEY "${PERPLEXITY_API_KEY_VALUE}")
string(REPLACE "\r" "" PERPLEXITY_API_KEY "${PERPLEXITY_API_KEY}")

# Read Google OAuth client values from .env
set(GOOGLE_CLIENT_ID_VALUE "")
string(REGEX MATCH "GOOGLE_CLIENT_ID[ \t]*=([^\n\r]*)" _GID_MATCH "${_ENV_TXT}")
if(_GID_MATCH)
  set(GOOGLE_CLIENT_ID_VALUE "${CMAKE_MATCH_1}")
endif()
string(REGEX REPLACE "^\"(.*)\"$" "\\1" GOOGLE_CLIENT_ID_VALUE "${GOOGLE_CLIENT_ID_VALUE}")
string(REPLACE "\r" "" GOOGLE_CLIENT_ID_VALUE "${GOOGLE_CLIENT_ID_VALUE}")

set(GOOGLE_CLIENT_SECRET_VALUE "")
string(REGEX MATCH "GOOGLE_CLIENT_SECRET[ \t]*=([^\n\r]*)" _GSEC_MATCH "${_ENV_TXT}")
if(_GSEC_MATCH)
  set(GOOGLE_CLIENT_SECRET_VALUE "${CMAKE_MATCH_1}")
endif()
string(REGEX REPLACE "^\"(.*)\"$" "\\1" GOOGLE_CLIENT_SECRET_VALUE "${GOOGLE_CLIENT_SECRET_VALUE}")
string(REPLACE "\r" "" GOOGLE_CLIENT_SECRET_VALUE "${GOOGLE_CLIENT_SECRET_VALUE}")

# Generate a header with build-time constants
set(GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED_INCLUDE_DIR}")
configure_file("${CMAKE_SOURCE_DIR}/include/Config.h.in" "${GENERATED_INCLUDE_DIR}/Config.h" @ONLY)
target_include_directories(genai_reader PRIVATE "${GENERATED_INCLUDE_DIR}")

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
# Versioned desktop entry filename to allow version-specific MIME associations
set(DESKTOP_BASENAME "GenAI_EBook_Reader-v${PROJECT_VERSION}-x86_64.AppImage.desktop")
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
    DEPENDS dist_linux #dist_windows
    COMMENT "Creating distribution artifacts"
    VERBATIM)

  add_custom_target(dist_linux
    DEPENDS genai_reader
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/bin"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/share/applications"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:genai_reader>" "${APPDIR}/usr/bin/"
    # Copy desktop file with versioned filename so default apps/mimeapps can target the exact release
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/resources/appimage/genai-reader.desktop" "${APPDIR}/usr/share/applications/${DESKTOP_BASENAME}"

    # Point linuxdeploy to the versioned desktop entry
    COMMAND ${CMAKE_COMMAND} -E env QMAKE="${QMAKE6_BIN}" "${LINUXDEPLOY_BIN}" --appdir "${APPDIR}" -e "${APPDIR}/usr/bin/genai_reader" -d "${APPDIR}/usr/share/applications/${DESKTOP_BASENAME}" -i "${APPDIR}/usr/share/icons/hicolor/256x256/apps/genai-reader.png" --output appimage --plugin qt 
    COMMAND ${CMAKE_COMMAND} -E rename "${CMAKE_BINARY_DIR}/GenAI_E-Book_Reader-x86_64.AppImage" "${DIST_DIR}/GenAI_EBook_Reader-v${PROJECT_VERSION}-x86_64.AppImage"

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Creating AppImage bundle in ${DIST_DIR}"
  )
  
endif()

# Custom target to create icons. This will be a dependency for the main executable.
add_custom_target(create_icons
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/share/icons/hicolor/32x32/apps"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/share/icons/hicolor/64x64/apps"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/share/icons/hicolor/128x128/apps"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/share/icons/hicolor/256x256/apps"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/share/icons/hicolor/512x512/apps"
    COMMAND ${CONVERT_BIN} "${CMAKE_SOURCE_DIR}/docs/imgs/logo-do-projeto.png" -resize 32x32! "${APPDIR}/usr/share/icons/hicolor/32x32/apps/genai-reader.png"
    COMMAND ${CONVERT_BIN} "${CMAKE_SOURCE_DIR}/docs/imgs/logo-do-projeto.png" -resize 64x64! "${APPDIR}/usr/share/icons/hicolor/64x64/apps/genai-reader.png"
    COMMAND ${CONVERT_BIN} "${CMAKE_SOURCE_DIR}/docs/imgs/logo-do-projeto.png" -resize 128x128! "${APPDIR}/usr/share/icons/hicolor/128x128/apps/genai-reader.png"
    COMMAND ${CONVERT_BIN} "${CMAKE_SOURCE_DIR}/docs/imgs/logo-do-projeto.png" -resize 256x256! "${APPDIR}/usr/share/icons/hicolor/256x256/apps/genai-reader.png"
    COMMAND ${CONVERT_BIN} "${CMAKE_SOURCE_DIR}/docs/imgs/logo-do-projeto.png" -resize 512x512! "${APPDIR}/usr/share/icons/hicolor/512x512/apps/genai-reader.png"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Creating application icons"
)

# Make the main target depend on icon creation
add_dependencies(genai_reader create_icons)

# Custom target to ensure AppStream metadata exists inside AppDir
add_custom_target(create_appdata
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/share/metainfo"
    # Remove previous file with mismatching name if present
    COMMAND ${CMAKE_COMMAND} -E rm -f "${APPDIR}/usr/share/metainfo/genai-reader.appdata.xml"
    # Copy metainfo with filename matching the component ID (avoids metainfo-filename-cid-mismatch)
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/metainfo/genai-reader.appdata.xml" "${APPDIR}/usr/share/metainfo/br.tec.rapport.genai-e-book-reader.appdata.xml"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Ensuring AppStream metadata is available in AppDir"
)

# Ensure the main target depends on appdata creation as well
add_dependencies(genai_reader create_appdata)

# Install rule for desktop integration
install(FILES resources/appimage/genai-reader.desktop DESTINATION share/applications)
install(DIRECTORY "${APPDIR}/usr/share/icons/hicolor/" DESTINATION share/icons)
install(FILES "${CMAKE_SOURCE_DIR}/metainfo/genai-reader.appdata.xml" DESTINATION share/metainfo RENAME br.tec.rapport.genai-e-book-reader.appdata.xml)

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

# ----------------------------------------------------------------------------
# AppStream validation (appstreamcli)
find_program(APPSTREAMCLI_BIN appstreamcli)
if(APPSTREAMCLI_BIN)
  add_custom_target(validate_appstream
    COMMAND ${APPSTREAMCLI_BIN} validate --pedantic "${CMAKE_SOURCE_DIR}/metainfo/genai-reader.appdata.xml"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Validating AppStream metainfo with appstreamcli"
    VERBATIM
  )
  # If distribution targets exist, make them depend on validation
  if(TARGET dist)
    add_dependencies(dist validate_appstream)
  endif()
  if(TARGET dist_linux)
    add_dependencies(dist_linux validate_appstream)
  endif()
else()
  message(WARNING "'appstreamcli' not found. Skipping AppStream validation. Install 'appstream' package to enable.")
endif()

# Convenience target: build dist and copy AppImage to user's Downloads
add_custom_target(github_release
    DEPENDS dist
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} git add -A
    # Allow clean working tree: if there is nothing to commit, proceed without failing
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} /bin/sh -c "git commit -m 'Release version ${PROJECT_VERSION}' || echo 'No changes to commit'"
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} git push
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} git tag -a "v${PROJECT_VERSION}" -m "Release version ${PROJECT_VERSION}" --force
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} git push origin "v${PROJECT_VERSION}" --force

    # Extract release notes from CHANGELOG.md
    COMMAND ${CMAKE_COMMAND} -E echo "Extracting release notes for v${PROJECT_VERSION}..."
    COMMAND ${CMAKE_COMMAND} -DRELEASE_VERSION=${PROJECT_VERSION} -DCMAKE_BINARY_DIR=${CMAKE_BINARY_DIR} -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ExtractChangelog.cmake"

    # Create GitHub release
    COMMAND ${GH_BIN} release create "v${PROJECT_VERSION}" --title "Release v${PROJECT_VERSION}" --notes-file "${CMAKE_BINARY_DIR}/release_notes.md"
    COMMAND ${GH_BIN} release upload "v${PROJECT_VERSION}" "${DIST_DIR}/GenAI_EBook_Reader-v${PROJECT_VERSION}-x86_64.AppImage"
    COMMAND ${GH_BIN} release upload "v${PROJECT_VERSION}" "${CMAKE_SOURCE_DIR}/APPIMAGE.md"

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Creating GitHub release and uploading artifacts for v${PROJECT_VERSION}"
    VERBATIM
)

add_custom_target(github_release_update
    DEPENDS local_release
    COMMAND ${GH_BIN} release upload "v${PROJECT_VERSION}" "${DIST_DIR}/GenAI_EBook_Reader-v${PROJECT_VERSION}-x86_64.AppImage" --clobber 
    COMMAND ${GH_BIN} release upload "v${PROJECT_VERSION}" "${CMAKE_SOURCE_DIR}/APPIMAGE.md" --clobber

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Updating GitHub release and uploading artifacts for v${PROJECT_VERSION}"
    VERBATIM
)

add_custom_target(local_release
  DEPENDS dist
  COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/Downloads"
  COMMAND ${CMAKE_COMMAND} -E remove "$ENV{HOME}/Downloads/GenAI_EBook_Reader-v${PROJECT_VERSION}-x86_64.AppImage"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DIST_DIR}/GenAI_EBook_Reader-v${PROJECT_VERSION}-x86_64.AppImage" "$ENV{HOME}/Downloads/"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Copying AppImage to ~/Downloads after building dist"
)
