cmake_minimum_required(VERSION 3.16)

project(GenAI_EBook_Reader VERSION 0.1.2 LANGUAGES C CXX)

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
  find_package(Qt6 COMPONENTS Widgets PdfWidgets QUIET)
  if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets QUIET)
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
    else()
      set(QT_PREFIX Qt5)
      set(QT_LIB Qt5::Widgets)
      # Qt5 Pdf not configured in this project by default
    endif()

    add_executable(genai_reader ${APP_SOURCES})

    target_include_directories(genai_reader PRIVATE include ${CMAKE_SOURCE_DIR}/src)

    target_link_libraries(genai_reader PRIVATE ${QT_LIB})
    if(DEFINED QT_PDF)
      target_link_libraries(genai_reader PRIVATE ${QT_PDF})
      target_compile_definitions(genai_reader PRIVATE HAVE_QT_PDF)
    endif()

    # Define to enable Qt-specific code paths
    target_compile_definitions(genai_reader PRIVATE USE_QT)

    # Enable AUTOMOC/AUTORCC/AUTOUIC for convenience
    set_target_properties(genai_reader PROPERTIES
      AUTOMOC ON
      AUTORCC ON
      AUTOUIC ON
    )

  else()
    message(WARNING "Qt not found. Building console placeholder. Set BUILD_WITH_QT=OFF to silence.")
  endif()
endif()

# Fallback: simple console app when Qt is unavailable
if(NOT TARGET genai_reader)
  add_executable(genai_reader ${APP_SOURCES})
  target_include_directories(genai_reader PRIVATE include ${CMAKE_SOURCE_DIR}/src)
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
