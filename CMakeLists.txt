cmake_minimum_required(VERSION 3.16)
project(EvePathfinder VERSION 1.0.0)


if(NOT DEFINED ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "Missing required environment variable CCP_EVE_PERFORCE_BRANCH_PATH")
elseif(NOT IS_ABSOLUTE $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH needs to be an absolute path")
elseif(NOT EXISTS $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH is pointing at a non-existing location $ENV{CCP_EVE_PERFORCE_BRANCH_PATH}")
elseif(NOT IS_DIRECTORY $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH needs to be a directory")
else()
    message(STATUS "Using CCP_EVE_PERFORCE_BRANCH_PATH located at $ENV{CCP_EVE_PERFORCE_BRANCH_PATH}")
endif()

file(TO_CMAKE_PATH "$ENV{CCP_EVE_PERFORCE_BRANCH_PATH}" BRANCH_ROOT_DIR)
list(INSERT CMAKE_MODULE_PATH 0 ${BRANCH_ROOT_DIR}/cmake)
include(CcpGlobalSettings)
include(CcpPackageConfigHelpers)

find_package(Python REQUIRED NO_CMAKE_PATH)
find_package(BlueExposure REQUIRED NO_CMAKE_PATH)
find_package(RadGameToolsTelemetry REQUIRED)
find_package(CcpCore REQUIRED NO_CMAKE_PATH)

add_library(EvePathfinder SHARED)
target_link_libraries(EvePathfinder PRIVATE BlueExposure)

target_sources(EvePathfinder PRIVATE
    EveDjikstrasGoal.cpp
    EveDjikstrasGoal.h
    EveDjikstrasGoal_Blue.cpp
    EveFloodFillGoal.cpp
    EveFloodFillGoal.h
    EveMap.cpp
    EveMap.h
    EveMap_Blue.cpp
    EveMapNodes.h
    EveMapPathfinderCache.cpp
    EveMapPathfinderCache.h
    EveMapPathfinderCache_Blue.cpp
    EvePathfinder.cpp
    EvePathfinder.h
    EveStandardFloodfillGoal.cpp
    EveStandardFloodfillGoal.h
    EveStandardFloodfillGoal_Blue.cpp
    IEvePathfinderGoal_Blue.cpp
    PResultBeResult.h
    stdafx.cpp
    stdafx.h
    testEvePathfinder.cpp
    Include/IEvePathfinderGoal.h
)
get_target_property(_SOURCES EvePathfinder SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
        PREFIX "Sources"
        FILES ${_SOURCES}
)

target_compile_definitions(EvePathfinder PRIVATE BLUEBUILD)

target_precompile_headers(EvePathfinder PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h)
target_include_directories(EvePathfinder PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(EvePathfinder PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../carbon/src)

if(APPLE)
    set_target_properties(
        EvePathfinder
        PROPERTIES
            PREFIX ""
            SUFFIX ".so"
    )
    target_link_libraries(EvePathfinder PRIVATE "-framework CoreFoundation -framework Cocoa")
endif()
set_target_properties(EvePathfinder PROPERTIES OUTPUT_NAME "_pyevepathfinder")

#install(TARGETS EvePathfinder)
# Provide an install target iff this is the top level project only
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Install rule to ensure that our runtime and linker files are in the expected, platform-specific folders
    install(
        TARGETS EvePathfinder
        EXPORT EvePathfinderTargets
        CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
        LIBRARY DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        ARCHIVE DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        RUNTIME DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
    )
    # Install rule for available public headers
    install(FILES ${PUBLIC_HEADER_FILES} DESTINATION include)

    message(STATUS "lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/")

    configure_ccp_vendor_config_file(
        TARGET EvePathfinder
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/EvePathfinderConfig.cmake"
        DEPENDENCIES ""
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/EvePathfinderConfig.cmake DESTINATION .)

     # Install python package
     install(DIRECTORY python/ DESTINATION python
     FILES_MATCHING
     PATTERN "*.py"
)
endif()