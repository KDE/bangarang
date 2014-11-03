project(Bangarang)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

find_package(ECM 1.1.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(BANGARANG_VERSION_MAJOR 3)
set(BANGARANG_VERSION_MINOR 0)
set(BANGARANG_VERSION_MICRO 0)
set(BANGARANG_VERSION ${BANGARANG_VERSION_MAJOR}.${BANGARANG_VERSION_MINOR}.${BANGARANG_VERSION_MICRO})
add_definitions(-DBANGARANG_VERSION="${BANGARANG_VERSION}")


include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core)
find_package(KF5 REQUIRED COMPONENTS CoreAddons I18n)

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
