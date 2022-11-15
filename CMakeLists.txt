project(EvePathfinder VERSION 1.0.0)

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

install(TARGETS EvePathfinder)
