
# all source files
set(BangarangSources
    app/main.cpp
)


# our executable target
add_executable(bangarang ${BangarangSources})

# libraries we need to link
target_link_libraries(bangarang
     Qt5::Widgets
     KF5::I18n
     KF5::CoreAddons
)

# targets to be installed
install(TARGETS bangarang ${INSTALL_TARGETS_DEFAULT_ARGS})

