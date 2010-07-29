set(BangarangNepomukWriterSources
    main.cpp
#    ../mediaitemmodel.cpp
    ../mediavocabulary.cpp
)

kde4_add_executable(bangarangnepomukwriter ${BangarangNepomukWriterSources})

target_link_libraries(bangarangnepomukwriter
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${NEPOMUK_LIBRARIES}
)

########### install files ###############

install(TARGETS bangarangnepomukwriter ${INSTALL_TARGETS_DEFAULT_ARGS})
