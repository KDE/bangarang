project(Bangarang)

set(WITHTRANSLATIONS YES)

find_package(KDE4 4.5.0 REQUIRED)
find_package(Nepomuk REQUIRED)

include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} "-DDISABLE_NEPOMUK_LEGACY=1")

find_package(Taglib REQUIRED)

include_directories(
    ${KDE4_INCLUDES}
    ${KROSS_INCLUDES}
    ${NEPOMUK_INCLUDE_DIR}
    ${QT_QTSCRIPT_INCLUDES}
    ${SOPRANO_INCLUDE_DIR}
    ${TAGLIB_INCLUDES}
)

add_subdirectory(icons)
add_subdirectory(src)

if (WITHTRANSLATIONS)
    find_package(Msgfmt REQUIRED)
    find_package(Gettext REQUIRED)
    add_subdirectory(po)
endif( WITHTRANSLATIONS)

########### install files ###############

install(FILES data/bangarang.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES data/bangarang_play_dvd.desktop data/bangarang_play_cd.desktop DESTINATION ${DATA_INSTALL_DIR}/solid/actions)
#install(FILES data/genrerc DESTINATION ${DATA_INSTALL_DIR}/bangarang)
#install(FILES data/thumbnails/Rock.png DESTINATION ${DATA_INSTALL_DIR}/bangarang/thumbnails/genres)
#install(FILES bangarangrc DESTINATION ${CONFIG_INSTALL_DIR})
