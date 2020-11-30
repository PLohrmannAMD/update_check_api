cmake_minimum_required(VERSION 3.11)

project(UpdateCheckerGUI)

# switch on the autogenerators needed for Qt. Includes the MetaObject Compiler (MOC),
# the UI compiler (UIC) and the resource compiler (RCC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

IF (WIN32)
    # get the location of the Visual Studio redist libraries - but don't create an install rule to install them
    # the libs will be copied manually as a post build step
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    include(InstallRequiredSystemLibraries)
ENDIF(WIN32)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(AFTER ../UpdateCheckerGUI)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

IF(UNIX)
    find_package(Threads)
ENDIF(UNIX)

# Add UpdateCheckAPI
add_subdirectory (${PROJECT_SOURCE_DIR}/../../UpdateCheckAPI ${CMAKE_CURRENT_BINARY_DIR}/UpdateCheckAPI)
include_directories(${UPDATECHECKAPI_INC_DIRS})


# List of all source files. It may be possible to have the build process call cmake to update the makefiles
# only when this file has changed (ie source files have been added or removed)


set( SOURCES
    "main.cpp"
    "MainWindow.cpp"
    "MainWindow.h"
    ${UPDATECHECKAPI_SRC}
    ${UPDATECHECKAPI_INC}
    ${UPDATECHECKAPI_QT_SRC}
    ${UPDATECHECKAPI_QT_INC}
    ${UPDATECHECKAPI_QT_UI}
)

# Specify output executable name. For windows, indicate this is a Windows application
# and not a console application
IF(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
ELSEIF(APPLE)
#    set(MACOSX_BUNDLE_ICON_FILE rgp.icns)
#    set(MACOSX_ICON MacOSX/rgp.icns)
#    set_source_files_properties(${MACOSX_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
#    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${MACOSX_ICON} ${SOURCES})
#    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_RPATH TRUE MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/MacOSX/MacOSXBundleInfo.plist.in)
ELSEIF(UNIX)
    add_executable(${PROJECT_NAME} ${SOURCES})
    # Setting RPATH here results in local build directory also being appended to RPATH
    # RPATH set instead by calling chrpath command line command for cleaner solution.
    # Correct cmake solution will likely require use of a separate make install
    # for packaging
    # Leaving these Commands here for future reference
    #    set_target_properties(${PROJECT_NAME} PROPERTIES
    #      BUILD_WITH_INSTALL_RPATH TRUE
    #      INSTALL_RPATH_USE_LINK_PATH TRUE
    #      INSTALL_RPATH "\$ORIGIN/qt/lib"
    #)
ENDIF(WIN32)

# CMAKE_<CONFIG>_POSTFIX isn't applied automatically to executable targets so apply manually
IF(CMAKE_DEBUG_POSTFIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
ENDIF(CMAKE_DEBUG_POSTFIX)
IF(CMAKE_RELEASE_POSTFIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX})
ENDIF(CMAKE_RELEASE_POSTFIX)

# executable file library dependency list
IF(WIN32)
    target_link_libraries(${PROJECT_NAME} Qt5::Widgets wsock32 winmm ${UPDATECHECKAPI_LIBS})
ELSEIF(UNIX)
    target_link_libraries(${PROJECT_NAME} Qt5::Widgets Threads::Threads ${UPDATECHECKAPI_LIBS})
ENDIF()

IF(APPLE)
    # Run macdeployqt if it can be found
    find_program(MACDEPLOYQT_EXECUTABLE NAMES macdeployqt)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        # macdeployqt expects to be passed the "${PROJECT_NAME}.app" directory rather than the final target
        COMMAND ${MACDEPLOYQT_EXECUTABLE} $<TARGET_FILE_DIR:${PROJECT_NAME}>/../../
    )
ENDIF(APPLE)

IF(WIN32)
    ## Not using windeployqt currently as it copies more files than necessary. Leaving the command
    ## here for future reference
    #
    # Run windeployqt if it can be found
    #    find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt)
    #    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    #        COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}> --no-translations --no-angle --no-opengl-sw --no-system-d3d-compiler --no-compiler-runtime --no-plugins
    #    )

    # Copy the QT files to the output directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
         # TBD - can windows use the same subdirectory layout as Linux (i.e. files in qt subfolder references via qt.conf)"
         COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/imageformats
         COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms
         COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/iconengines
         COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:${PROJECT_NAME}>
         COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:${PROJECT_NAME}>
         COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:${PROJECT_NAME}>
         COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin> $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms
     )

    # Copy the VisualStudio redist files
    # the list of all redist files is contained in the variable CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS. We only want to copy
    # a subset of these so we extract the directory from the first entry and then manually copy the files we want
    foreach(VSLIB ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
        get_filename_component(VSREDISTDIR ${VSLIB} DIRECTORY)
        message("Visual Studio redistributable files directory = ${VSREDISTDIR}")
        break()
    endforeach()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "copying Visual Studio redist files from ${VSREDISTDIR} to $<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VSREDISTDIR}/msvcp140.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VSREDISTDIR}/vcruntime140.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    #
    # Custom commands that will only execute in debug builds.
    # These are of the form "IF:Condition,then-action1&&action2,else-action"
    #

    # copy RGP-Updates.json to VS local debug directory.
    string ( APPEND _cmd_copy_updates_to_vs_wd
        "$<IF:$<CONFIG:Debug>,"
            "${CMAKE_COMMAND};-E;echo;copying UpdateCheckerGUI-Updates.json to VisualStudio working directory;&&;"
            "${CMAKE_COMMAND};-E;copy_if_different;${CMAKE_CURRENT_SOURCE_DIR}/UpdateCheckerGUI-Updates.json;$(ProjectDir),"
            ""
        ">"
    )

    # copy RGP-Updates.json to binary output directory.
    string ( APPEND _cmd_copy_updates_to_output_dir
        "$<IF:$<CONFIG:Debug>,"
            "${CMAKE_COMMAND};-E;echo;copying UpdateCheckerGUI-Updates.json to output directory;&&;"
            "${CMAKE_COMMAND};-E;copy_if_different;${CMAKE_CURRENT_SOURCE_DIR}/UpdateCheckerGUI-Updates.json;$(OutputPath),"
            ""
        ">"
    )

    # Add the Debug-Only custom commands.
    # This functionality requires CMake 3.8 or higher.
    add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${_cmd_copy_updates_to_vs_wd}"
        COMMAND "${_cmd_copy_updates_to_output_dir}"
        COMMAND_EXPAND_LISTS
    )

ENDIF(WIN32)

IF(UNIX AND NOT APPLE)
    # define some variables for the source and destination of the QT lib copy
    set(QT_LIB_SRC "$<TARGET_FILE_DIR:Qt5::Core>")
    set(QT_LIB_DST "$<TARGET_FILE_DIR:${PROJECT_NAME}>/qt/lib")
    set(QT_PLATFORM_SRC "$<TARGET_FILE_DIR:Qt5::QXcbIntegrationPlugin>")
    set(QT_PLATFORM_DST "$<TARGET_FILE_DIR:${PROJECT_NAME}>/qt/plugins/platforms")
    set(QT_IMAGEFORMATS_SRC "$<TARGET_FILE_DIR:Qt5::QSvgPlugin>")
    set(QT_IMAGEFORMATS_DST "$<TARGET_FILE_DIR:${PROJECT_NAME}>/qt/plugins/imageformats")
    set(QT_ICONENGINES_SRC "$<TARGET_FILE_DIR:Qt5::QSvgIconPlugin>")
    set(QT_ICONENGINES_DST "$<TARGET_FILE_DIR:${PROJECT_NAME}>/qt/plugins/imageformats")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E echo "copying QT libs from ${QT_LIB_SRC} to ${QT_LIB_DST}"
         COMMAND ${CMAKE_COMMAND} -E make_directory ${QT_LIB_DST}
         COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/qt/plugins/iconengines
         COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/qt/plugins/imageformats
         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_LIB_SRC}/libQt5Core.so.5    ${QT_LIB_DST}
         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_LIB_SRC}/libQt5Gui.so.5     ${QT_LIB_DST}
         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_LIB_SRC}/libQt5Widgets.so.5 ${QT_LIB_DST}
         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_LIB_SRC}/libQt5XcbQpa.so.5  ${QT_LIB_DST}
         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_LIB_SRC}/libQt5DBus.so.5    ${QT_LIB_DST}
         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_LIB_SRC}/libicudata.so.56   ${QT_LIB_DST}
         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_LIB_SRC}/libicui18n.so.56   ${QT_LIB_DST}
         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_LIB_SRC}/libicuuc.so.56     ${QT_LIB_DST}
         COMMAND ${CMAKE_COMMAND} -E echo "copying QT platform plugins from ${QT_PLATFORM_SRC} to ${QT_PLATFORM_DST}"
         COMMAND ${CMAKE_COMMAND} -E make_directory ${QT_PLATFORM_DST}
         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_PLATFORM_SRC}/libqxcb.so    ${QT_PLATFORM_DST}
         COMMAND ${CMAKE_COMMAND} -E echo "copying qt.conf to $<TARGET_FILE_DIR:${PROJECT_NAME}>"
         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../build/qt.conf $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    # Call chrpath on system to override executable file RPATH for both comgr and qt.
    find_program(CHRPATH_EXECUTABLE NAMES chrpath)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "trying chrpath"
        COMMAND ${CHRPATH_EXECUTABLE} -r "\\$$ORIGIN/qt/lib:\\$$ORIGIN/lib:" $<TARGET_FILE:${PROJECT_NAME}>
    )

ENDIF(UNIX AND NOT APPLE)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "copying rtda to output directory"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RTDA_PATH} $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

IF(WIN32)
# Create Visual Studio filters so that the source files in the project match the directory structure
foreach(source IN LISTS SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
ENDIF(WIN32)
