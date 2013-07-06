set(BangarangNepomukWriterSources
    main.cpp
    nepomukwriter.cpp
#    ../mediaitemmodel.cpp
    ../mediavocabulary.cpp
    ../mediaquery.cpp
)

kde4_add_executable(bangarangnepomukwriter ${BangarangNepomukWriterSources})

target_link_libraries(bangarangnepomukwriter
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${NEPOMUK_CORE_LIBRARY}
    ${SOPRANO_LIBRARIES}
)

########### install files ###############

install(TARGETS bangarangnepomukwriter ${INSTALL_TARGETS_DEFAULT_ARGS})
