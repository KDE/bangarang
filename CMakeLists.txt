PROJECT(Bangarang)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )
set( WITHTRANSLATIONS:BOOL YES )
FIND_PACKAGE(KDE4 REQUIRED)
FIND_PACKAGE(Nepomuk REQUIRED)
FIND_PACKAGE(KdeMultimedia REQUIRED)

INCLUDE(KDE4Defaults)

ADD_DEFINITIONS( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )

find_package(Taglib REQUIRED)

INCLUDE_DIRECTORIES( 
    ${KDE4_INCLUDES} 
    ${KDEMULTIMEDIA_INCLUDE_DIR} 
    ${NEPOMUK_INCLUDE_DIR} 
    ${TAGLIB_INCLUDES} 
    ${TAGLIB_INCLUDES}/.. 
    /usr/include/taglib 
)

add_subdirectory( icons )
add_subdirectory( src )

find_package(Msgfmt REQUIRED)
find_package(Gettext REQUIRED)
add_subdirectory( translations )

########### install files ###############

install(FILES data/bangarang.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES data/bangarang_play_dvd.desktop data/bangarang_play_cd.desktop DESTINATION ${DATA_INSTALL_DIR}/solid/actions)
#install(FILES <bangarangDataFilesHere> DESTINATION ${DATA_INSTALL_DIR}/bangarang)
#install(FILES bangarangrc DESTINATION ${CONFIG_INSTALL_DIR})
