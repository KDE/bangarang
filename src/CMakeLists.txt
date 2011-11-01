add_subdirectory(platform/nepomukwriter)

set(BangarangSources
    app/main.cpp
    app/common/actionsmanager.cpp
    app/common/bangarangapplication.cpp
    app/common/bangarangnotifieritem.cpp
    app/common/dbusobjects.cpp
    app/common/flickcharm.cpp
    app/common/mainwindow.cpp
    app/common/mediaitemdelegate.cpp
    app/common/mediaview.cpp
    app/common/ratingdelegate.cpp
    app/common/sensiblewidgets.cpp
    app/common/starrating.cpp
    app/common/artworkpainter.cpp
    app/medialists/audiolistsstack.cpp
    app/medialists/medialistsettings.cpp
    app/medialists/medialistsmanager.cpp
    app/medialists/infobox.cpp
    app/medialists/infomanager.cpp
    app/medialists/infoitemdelegate.cpp
    app/medialists/infoitemview.cpp
    app/medialists/savedlistsmanager.cpp
    app/medialists/videolistsstack.cpp
    app/nowplaying/audiosettings.cpp
    app/nowplaying/bangarangvideowidget.cpp
    app/nowplaying/bookmarksmanager.cpp
    app/nowplaying/nowplayingdelegate.cpp
    app/nowplaying/nowplayingmanager.cpp
    app/nowplaying/nowplayingview.cpp
    app/nowplaying/playlistview.cpp
    app/nowplaying/videosettings.cpp
    platform/dbpediaquery.cpp
    platform/devicemanager.cpp
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
    platform/infofetchers/doubaninfofetcher.cpp
    platform/infofetchers/feedinfofetcher.cpp
    platform/infofetchers/filenameinfofetcher.cpp
    platform/infofetchers/infofetcher.cpp
    platform/infofetchers/lastfminfofetcher.cpp
    platform/infofetchers/tmdbinfofetcher.cpp
    platform/infofetchers/tvdbinfofetcher.cpp
    platform/listengines/ampachelistengine.cpp
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
    platform/utilities/sha256.cpp
    platform/utilities/thread.cpp
    platform/utilities/typechecks.cpp
)

kde4_add_ui_files(BangarangSources app/ui/mainwindow.ui app/ui/audiolistsstack.ui app/ui/videolistsstack.ui)

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
