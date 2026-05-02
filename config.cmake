
option(GIBS_LTO "Enable link time optimization" OFF)
option(GIBS_UNITY_BUILD "Enable unity (jumbo) builds, which offer much faster compilation times" OFF)
option(GIBS_CCACHE "Enable ccache - much faster repeating builds" OFF)
option(GIBS_MOLD "Enable mold linker - much faster linker" OFF)
option(GIBS_ASAN "Enable address sanitizer" OFF)
option(GIBS_STRICT "Treat warnings errors, plus enforce other, related checks" ON)

# Enable LTO (link time optimization)
if (GIBS_LTO)
  set(LINK_TIME_OPTIMIZATION ON)
  message("Building with LTO enabled")
endif ()

# Enable unity builds
if (GIBS_UNITY_BUILD)
  set(UNITY_BUILD_VALUE ON)
  set(UNITY_BUILD_MODE_VALUE BATCH)
  set(UNITY_BUILD_BATCH_SIZE_VALUE 16)
  message("Building with unity (jumbo) build enabled")
endif ()

if (GIBS_CCACHE)
  set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
  message("Building using ccache")
endif ()

if (GIBS_MOLD)
  # See:https://github.com/rui314/mold
  # and: https://gist.github.com/MawKKe/b8af6c1555f1c7aa4c2760350ed97fff
  set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=mold")
  set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=mold")
  message("Building using mold linker")
endif ()

if (GIBS_ASAN)
  if (WIN32)
    if (MSVC)
      message("No ASAN for you, my boy!") # But! modern MSVC does support asan!
    endif ()
  else ()
    add_definitions("-fsanitize=address" "-fno-optimize-sibling-calls" "-fsanitize-address-use-after-scope" "-fno-omit-frame-pointer")
    add_link_options("-fsanitize=address" "-fno-optimize-sibling-calls" "-fsanitize-address-use-after-scope" "-fno-omit-frame-pointer")
    message("Building using asan")
  endif ()
endif ()

if (WIN32)
  if (MSVC)
    if (${ENABLE_STRICT})
      add_definitions("/utf-8" "/WX" "/wd4573" "/W3" "/bigobj")
    else()
      add_definitions("/utf-8" "/bigobj")
    endif()
  endif()
elseif(APPLE)
  if (${ENABLE_STRICT})
    add_definitions("-Werror" "-Wall" "-Wextra" "-Wconversion" "-Wno-error=switch")
  endif()
else()
  if (${ENABLE_STRICT})
    add_definitions("-Werror" "-Wall" "-Wextra" "-Wconversion"
    "-Wno-error=deprecated-declarations" "-Wsign-conversion"
    "-pedantic-errors" "-Wno-error=nonnull")
  endif()
endif()
