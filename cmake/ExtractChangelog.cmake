# This script is executed by CMake to extract release notes from CHANGELOG.md

# Determine project root from this script's directory (script is in <root>/cmake)
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

# Read the entire CHANGELOG.md file (located at the project root)
file(READ "${PROJECT_ROOT}/CHANGELOG.md" CHANGELOG_CONTENT)

# Determine release version: prefer RELEASE_VERSION passed via -D, fallback to PROJECT_VERSION
if(DEFINED RELEASE_VERSION AND NOT RELEASE_VERSION STREQUAL "")
  set(_VERSION "${RELEASE_VERSION}")
elseif(DEFINED PROJECT_VERSION AND NOT PROJECT_VERSION STREQUAL "")
  set(_VERSION "${PROJECT_VERSION}")
else()
  message(FATAL_ERROR "Release version not provided. Pass -DRELEASE_VERSION=<version> when invoking this script.")
endif()

# Find the section that starts with '## [<version>]' and capture until the next '## [' or end of file.
set(MARKER "## [${_VERSION}]")
string(FIND "${CHANGELOG_CONTENT}" "${MARKER}" START_POS)
if(START_POS EQUAL -1)
  message(FATAL_ERROR "Could not find release notes for version ${_VERSION} in CHANGELOG.md")
endif()

# Substring from the start of this version section to the end
string(SUBSTRING "${CHANGELOG_CONTENT}" ${START_POS} -1 REMAINDER)

# Look for the beginning of the next section header
string(FIND "${REMAINDER}" "\n## [" NEXT_REL_POS)
if(NEXT_REL_POS EQUAL -1)
  set(RELEASE_NOTES "${REMAINDER}")
else()
  string(SUBSTRING "${REMAINDER}" 0 ${NEXT_REL_POS} RELEASE_NOTES)
endif()

# Trim leading and trailing whitespace/newlines from the captured section
string(STRIP "${RELEASE_NOTES}" RELEASE_NOTES)

# Write the extracted notes to a temporary file in the build directory
# In script mode, CMAKE_BINARY_DIR may not be defined; prefer current working directory if not
if(DEFINED CMAKE_BINARY_DIR AND NOT CMAKE_BINARY_DIR STREQUAL "")
  set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/release_notes.md")
else()
  set(OUTPUT_FILE "${CMAKE_CURRENT_LIST_DIR}/../build/release_notes.md")
endif()
file(WRITE "${OUTPUT_FILE}" "${RELEASE_NOTES}")

message(STATUS "Release notes for v${_VERSION} extracted to ${OUTPUT_FILE}")
