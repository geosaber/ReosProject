
DEFINES += QWT_DLL
INCLUDEPATH +=$${QWT_INCLUDE_PATH}


CONFIG(debug, debug|release){
    LIBS +=-L$${QWT_LIB_FILE} -lqwtd
} else {
    LIBS +=-L$${QWT_LIB_FILE} -lqwt
}




