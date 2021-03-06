# Reos GPL v2.0
# Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)

SET(REOS_GUI_SOURCES
    reosaboutwidget.cpp
    reoslanguageselectionwidget.cpp
    reosmainwindow.cpp
    reosmessagebox.cpp
    reosstartingwidget.cpp
    reosversionmessagebox.cpp
)

SET(REOS_GUI_HEADERS
    reosaboutwidget.h
    reoslanguageselectionwidget.h
    reosmainwindow.h
    reosmessagebox.h
    reosstartingwidget.h
    reosversionmessagebox.h
)

ADD_LIBRARY(reosGui SHARED
    ${REOS_GUI_HEADERS}
    ${REOS_GUI_SOURCES}
  )

TARGET_LINK_LIBRARIES(reosGui PUBLIC
        ${Qt5Xml_LIBRARIES}
        ${Qt5Core_LIBRARIES}
        ${Qt5Gui_LIBRARIES}
        ${Qt5Widgets_LIBRARIES}
        ${Qt5Svg_LIBRARIES}
        ${Qt5PrintSupport_LIBRARIES}
        ${Qt5Network_LIBRARIES}
        ${Qt5Sql_LIBRARIES}
        ${Qt5Concurrent_LIBRARIES}
        reosCore
)

IF(WIN32)
  INSTALL(TARGETS reosGui RUNTIME DESTINATION lib)
ELSE(WIN32)
  INSTALL(TARGETS reosGui LIBRARY DESTINATION lib)
ENDIF(WIN32)

INCLUDE_DIRECTORIES(
    ${CMAKE_BINARY_DIR}/src/ui
    ${CMAKE_SOURCE_DIR}/src/core
)


#TARGET_INCLUDE_DIRECTORIES(reosGui
#    PUBLIC
#    $<INSTALL_INTERFACE:include>
#    PRIVATE
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
#  )



# INSTALL HEADER
INSTALL(FILES ${REOS_GUI_HEADERS} DESTINATION include)
