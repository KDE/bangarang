
# all source files
set(BangarangSources
    app/main.cpp
    platform/mediaitemmodel.cpp
    platform/listengines/listengine.cpp
    platform/listengines/listenginefactory.cpp
    platform/downloader.cpp
    platform/medialistcache.cpp
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
)

# targets to be installed
install(TARGETS bangarang ${INSTALL_TARGETS_DEFAULT_ARGS})

