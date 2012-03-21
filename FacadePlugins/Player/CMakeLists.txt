project(PlayerPlugin)

set(HEADER_FILES
        PlayerControllerEditor.h
)

set(SOURCE_FILES
        PlayerControllerEditor.cpp
)

include_directories(${DEPENDENCIES_INCLUDES})
include_directories(${OGITOR_INCLUDES})
message(STATUS "${OGITOR_INCLUDES}")

include_directories(.)
link_directories(${OGITOR_LIBPATH})
link_directories(${DEPENDENCIES_LIBPATH})

link_libraries(${OGRE_LIBRARIES} Ogitor)
add_library(PlayerPlugin SHARED ${HEADER_FILES} ${SOURCE_FILES})
#target_link_libraries(OgreMain Ogitor)
set(LIBRARY_OUTPUT_PATH ../lib)

#set_target_properties(PlayerPlugin PROPERTIES INSTALL_RPATH ${OGRE_INSTALL_DIR}/lib/OGRE)
set_target_properties(PlayerPlugin PROPERTIES COMPILE_DEFINITIONS PLUGIN_EXPORT)
set_target_properties(PlayerPlugin PROPERTIES SOLUTION_FOLDER Plugins)

if(UNIX AND NOT APPLE)
        add_custom_command(TARGET PlayerPlugin PRE_BUILD
                COMMAND mkdir -p ../../RunPath/Plugins)
        add_custom_command(TARGET PlayerPlugin POST_BUILD
                COMMAND cp -r ../lib/libPlayerPlugin.* ../../RunPath/Plugins/)
endif(UNIX AND NOT APPLE)

if(WIN32)
        add_custom_command(TARGET PlayerPlugin PRE_BUILD
                COMMAND if not exist ..\\..\\RunPath\\Plugins mkdir ..\\..\\RunPath\\Plugins)
        add_custom_command(TARGET PlayerPlugin POST_BUILD
                COMMAND copy \"$(TargetPath)\" ..\\..\\RunPath\\Plugins & if exist ..\\..\\RunPath\\Plugins\\PlayerPlugin_d.dll move ..\\..\\RunPath\\Plugins\\PlayerPlugin_d.dll ..\\..\\RunPath\\Plugins\\debug)

        install(TARGETS PlayerPlugin
                RUNTIME DESTINATION Plugins
                CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
endif(WIN32)

IF(UNIX AND NOT APPLE)
    install(TARGETS PlayerPlugin
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/share/qtOgitor/Plugins
        CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
ENDIF(UNIX AND NOT APPLE)
