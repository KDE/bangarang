
# all source files
set(BangarangSources
    app/main.cpp
    platform/mediaitemmodel.cpp
    platform/listengines/listengine.cpp
    platform/listengines/listenginefactory.cpp
    platform/listengines/filelistengine.cpp
    platform/listengines/medialistsengine.cpp
    platform/listengines/cachelistengine.cpp
    platform/listengines/savedlistsengine.cpp
    platform/downloader.cpp
    platform/devicemanager.cpp
    platform/medialistcache.cpp
    platform/utilities/artwork.cpp
    platform/utilities/filetags.cpp
    platform/utilities/general.cpp
    platform/utilities/mediaitems.cpp
    platform/utilities/sha256.cpp
    platform/utilities/thread.cpp
    platform/utilities/typechecks.cpp
)


# our executable target
add_executable(bangarang ${BangarangSources})

# libraries we need to link
target_link_libraries(bangarang
     Qt5::Widgets
     KF5::I18n
     KF5::CoreAddons
     KF5::KIOCore 
     KF5::KIOFileWidgets 
     KF5::KIOWidgets 
     KF5::KIONTLM
     KF5::IconThemes
     Phonon::phonon4qt5
     tag
)

# targets to be installed
install(TARGETS bangarang ${INSTALL_TARGETS_DEFAULT_ARGS})

