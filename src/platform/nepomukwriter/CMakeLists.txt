PROJECT(Bangarang)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

FIND_PACKAGE(KDE4 REQUIRED)
FIND_PACKAGE(Nepomuk REQUIRED)

INCLUDE(KDE4Defaults)

ADD_DEFINITIONS( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )

INCLUDE_DIRECTORIES( 
    ${KDE4_INCLUDES} 
    ${KDEMULTIMEDIA_INCLUDE_DIR} 
    ${NEPOMUK_INCLUDE_DIR} 
)

SET(BangarangNepomukWriterSources 
    main.cpp 
#    ../mediaitemmodel.cpp
    ../mediavocabulary.cpp
)

KDE4_ADD_EXECUTABLE(bangarangnepomukwriter ${BangarangNepomukWriterSources})

TARGET_LINK_LIBRARIES(bangarangnepomukwriter 
    ${NEPOMUK_LIBRARIES} 
    ${KDEMULTIMEDIA_LIBRARIES} 
    ${SOPRANO_LIBRARIES} 
)

########### install files ###############

install(TARGETS bangarangnepomukwriter ${INSTALL_TARGETS_DEFAULT_ARGS})
