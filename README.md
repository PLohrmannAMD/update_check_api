# UpdateCheckAPI

The UpdateCheckAPI repository provides utility source code for AMD Tools to check for new releases available through GitHub's Release API. The code also provides a Qt-based dialog that can be used to display the results of the check for updates.

## Usage:

Projects that want to utilize UpdateCheckAPI can do so using CMake's add_subdirectory(<path to UpdateCheckAPI>) command. UpdateCheckAPI contains a CMakeLists.txt which defines several other cached variables that identify the various files that are necessary to use UpdateCheckAPI:

* UPDATECHECKAPI_SRC (source files)
* UPDATECHECKAPI_INC (header files)
* UPDATECHECKAPI_INC_DIRS (additional include directories)
* UPDATECHECKAPI_LIBS (required libraries)
* UPDATECHECKAPI_LIB_DIRS (additional library directories)

Additional CMake variables are also defined to utilize the Qt widgets:

* UPDATECHECKAPI_QT_SRC (source files which reference Qt components)
* UPDATECHECKAPI_QT_INC (header files which reference Qt components)
* UPDATECHECKAPI_QT_UI (ui files for the new widgets)

Also, the UpdateCheckAPI utilizes an executabled named AMDToolsDownloader to download files from the internet. This needs to copied into the applications working directory. To simplify copying the executable, its platform-specific path is cached in the CMake variable:

* AMDTOOLSDOWNLOADER_PATH (Path to the platform-specific AMDToolsDownloader executable)

## Release Notes:

Version 1.0
* Initial release
