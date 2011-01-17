add_subdirectory(platform/nepomukwriter)

set(BangarangSources
    actionsmanager.cpp
    audiosettings.cpp
    bangarangapplication.cpp
    bangarangnotifieritem.cpp
    bookmarksmanager.cpp
    dbusobjects.cpp
    mainwindow.cpp
    main.cpp
    mediaitemdelegate.cpp
    medialistsettings.cpp
    mediaview.cpp
    nowplayingdelegate.cpp
    nowplayingview.cpp
    playlistview.cpp
    infobox.cpp
    infomanager.cpp
    infoitemdelegate.cpp
    infoitemview.cpp
    ratingdelegate.cpp
    savedlistsmanager.cpp
    sensiblewidgets.cpp
    starrating.cpp
    videosettings.cpp
    platform/bangarangvideowidget.cpp
    platform/dbpediaquery.cpp
    platform/downloader.cpp
    platform/infoitemmodel.cpp
    platform/mediaitemmodel.cpp
    platform/mediaindexer.cpp
    platform/mediavocabulary.cpp
    platform/mediaquery.cpp
    platform/medialistcache.cpp
    platform/ontologyupdater.cpp
    platform/playlist.cpp
    platform/infofetchers/dbpediainfofetcher.cpp
    platform/infofetchers/feedinfofetcher.cpp
    platform/infofetchers/filenameinfofetcher.cpp
    platform/infofetchers/infofetcher.cpp
    platform/infofetchers/lastfminfofetcher.cpp
    platform/infofetchers/tmdbinfofetcher.cpp
    platform/infofetchers/tvdbinfofetcher.cpp
    platform/listengines/audioclipslistengine.cpp
    platform/listengines/audiostreamlistengine.cpp
    platform/listengines/cachelistengine.cpp
    platform/listengines/cdlistengine.cpp
    platform/listengines/dvdlistengine.cpp
    platform/listengines/feedlistengine.cpp
    platform/listengines/filelistengine.cpp
    platform/listengines/listengine.cpp
    platform/listengines/listenginefactory.cpp
    platform/listengines/medialistsengine.cpp
    platform/listengines/musiclistengine.cpp
    platform/listengines/nepomuklistengine.cpp
    platform/listengines/semanticslistengine.cpp
    platform/listengines/savedlistsengine.cpp
    platform/listengines/taglistengine.cpp
    platform/listengines/videolistengine.cpp
    platform/utilities/artwork.cpp
    platform/utilities/filetags.cpp
    platform/utilities/general.cpp
    platform/utilities/mediaitems.cpp
    platform/utilities/thread.cpp
    platform/utilities/typechecks.cpp
)

kde4_add_ui_files(BangarangSources mainwindow.ui)

kde4_add_executable(bangarang ${BangarangSources})

kde4_add_app_icon(BangarangSources "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*.png")

# Executable
target_link_libraries(bangarang
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_PHONON_LIBS}
    ${KDE4_KFILE_LIBS}
    ${KDE4_KROSSCORE_LIBS}
    ${KDE4_SOLID_LIBS}
    ${NEPOMUK_LIBRARIES}
    ${SOPRANO_LIBRARIES}
    ${TAGLIB_LIBRARIES}
    tag
)


########### install files ###############

install(TARGETS bangarang ${INSTALL_TARGETS_DEFAULT_ARGS})
# install(TARGETS  krossmodulebangarangmod DESTINATION ${PLUGIN_INSTALL_DIR}) # modules one might want to link against in c++ plugins (using dynamic libload magic)
#install(FILES <bangarangDataFilesHere> DESTINATION ${DATA_INSTALL_DIR}/bangarang) # We might need that once we can load plugins
#install(FILES bangarangrc DESTINATION ${CONFIG_INSTALL_DIR})
kde4_install_icons(${ICON_INSTALL_DIR})
