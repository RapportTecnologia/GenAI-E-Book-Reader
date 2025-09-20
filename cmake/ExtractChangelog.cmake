# This script is executed by CMake to extract release notes from CHANGELOG.md

# Read the entire CHANGELOG.md file
file(READ "${CMAKE_SOURCE_DIR}/CHANGELOG.md" CHANGELOG_CONTENT)

# Define the regex to find the section for the current project version
# It starts with '## [version]' and captures everything until the next '## [' or the end of the file.
set(REGEX "##\\[${PROJECT_VERSION}\\]([\\s\\S]*?)(?=(##\\[)|(----)|$"))

# Execute the regex
string(REGEX MATCH "${REGEX}" RELEASE_NOTES "${CHANGELOG_CONTENT}")

if(NOT RELEASE_NOTES)
    message(FATAL_ERROR "Could not find release notes for version ${PROJECT_VERSION} in CHANGELOG.md")
endif()

# The regex captures the title line (e.g., '## [0.1.7] - 2025-09-20'), we can remove it if desired
# or just use the content as is. For GitHub releases, keeping it is fine.

# Write the extracted notes to a temporary file in the build directory
set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/release_notes.md")
file(WRITE "${OUTPUT_FILE}" "${RELEASE_NOTES}")

message(STATUS "Release notes for v${PROJECT_VERSION} extracted to ${OUTPUT_FILE}")
