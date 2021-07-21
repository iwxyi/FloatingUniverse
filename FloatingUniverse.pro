QT       += core gui network websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

RC_FILE += resources/resource.rc

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += ENABLE_TRAY ENABLE_SHORTCUT

contains(DEFINES, ENABLE_SHORTCUT) {
    include($$PWD/third_party/qxtglobalshortcut5/qxt.pri)
}else{
    message("shortcuts not support")
}

INCLUDEPATH += third_party/color_octree/\
    third_party/facile_menu/\
    third_party/interactive_buttons/\
    third_party/smooth_scroll/\
    third_party/utils/\
    third_party/\
    global/\
    widgets/\
	mainwindow/

SOURCES += \
    global/usettings.cpp \
    panel/carditem.cpp \
    panel/customedit.cpp \
    panel/icontextitem.cpp \
    panel/imageitem.cpp \
    panel/longtextitem.cpp \
    panel/moveablewidget.cpp \
    panel/panelitembase.cpp \
    panel/resizeableitembase.cpp \
    panel/universepanel.cpp \
    third_party/color_octree/coloroctree.cpp \
    third_party/color_octree/imageutil.cpp \
    third_party/facile_menu/facilemenu.cpp \
    third_party/facile_menu/facilemenuitem.cpp \
    third_party/interactive_buttons/interactivebuttonbase.cpp \
    main.cpp \
    mainwindow/mainwindow.cpp \
    global/runtime.cpp \
    third_party/interactive_buttons/watercirclebutton.cpp \
    third_party/qtsingleapplication/qtlocalpeer.cpp \
    third_party/qtsingleapplication/qtlockedfile.cpp \
    third_party/qtsingleapplication/qtlockedfile_unix.cpp \
    third_party/qtsingleapplication/qtlockedfile_win.cpp \
    third_party/qtsingleapplication/qtsingleapplication.cpp \
    third_party/qtsingleapplication/qtsinglecoreapplication.cpp \
    third_party/sapid_switch/boundaryswitchbase.cpp \
    third_party/sapid_switch/lovelyheartswitch.cpp \
    third_party/sapid_switch/normalswitch.cpp \
    third_party/sapid_switch/sapidswitchbase.cpp \
    third_party/slim_scroll_bar/slimscrollbar.cpp \
    third_party/slim_scroll_bar/slimscrollbarpopup.cpp \
    third_party/smooth_scroll/smoothlistwidget.cpp \
    third_party/smooth_scroll/smoothscrollarea.cpp \
    third_party/utils/fileutil.cpp \
    third_party/utils/stringutil.cpp \
    third_party/utils/textinputdialog.cpp \
    widgets/anicirclelabel.cpp \
    widgets/aninumberlabel.cpp \
    mainwindow/settingscontroller.cpp \
    mainwindow/settingsitemlistbox.cpp

HEADERS += \
    global/accountinfo.h \
    global/defines.h \
    global/signaltransfer.h \
    global/usettings.h \
    panel/carditem.h \
    panel/customedit.h \
    panel/icontextitem.h \
    panel/imageitem.h \
    panel/longtextitem.h \
    panel/moveablewidget.h \
    panel/panelitembase.h \
    panel/resizeableitembase.h \
    panel/universepanel.h \
    resources/resource.rc \
    third_party/interactive_buttons/watercirclebutton.h \
    third_party/qtsingleapplication/QtLockedFile \
    third_party/qtsingleapplication/QtSingleApplication \
    third_party/qtsingleapplication/qtlocalpeer.h \
    third_party/qtsingleapplication/qtlockedfile.h \
    third_party/qtsingleapplication/qtsingleapplication.h \
    third_party/qtsingleapplication/qtsinglecoreapplication.h \
    third_party/sapid_switch/boundaryswitchbase.h \
    third_party/sapid_switch/lovelyheartswitch.h \
    third_party/sapid_switch/normalswitch.h \
    third_party/sapid_switch/sapidswitchbase.h \
    third_party/slim_scroll_bar/slimscrollbar.h \
    third_party/slim_scroll_bar/slimscrollbarpopup.h \
    third_party/smooth_scroll/smoothlistwidget.h \
    third_party/smooth_scroll/smoothscrollarea.h \
    third_party/smooth_scroll/smoothscrollbean.h \
    third_party/utils/mysettings.h \
    third_party/utils/netimageutil.h \
    third_party/color_octree/coloroctree.h \
    third_party/third_party/color_octree/imageutil.h \
    third_party/facile_menu/facilemenu.h \
    third_party/facile_menu/facilemenuitem.h \
    third_party/interactive_buttons/interactivebuttonbase.h \
    mainwindow/mainwindow.h \
    global/runtime.h \
    third_party/utils/dlog.h \
    third_party/utils/fileutil.h \
    third_party/utils/myjson.h \
    third_party/utils/netutil.h \
    third_party/utils/pinyinutil.h \
    third_party/utils/stringutil.h \
    third_party/utils/textinputdialog.h \
    widgets/anicirclelabel.h \
    widgets/aninumberlabel.h \
    widgets/clicklabel.h \
    widgets/customtabstyle.h \
    widgets/lefttabwidget.h \
    mainwindow/settingscontroller.h \
    mainwindow/settingsitemlistbox.h

FORMS += \
    mainwindow/mainwindow.ui \
    mainwindow/settingscontroller.ui \
    third_party/utils/textinputdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    README.md \
    qxtglobalshortcut5/qxt.pri \
    resources/icons/account.png \
    resources/icons/application.png \
    resources/icons/auxiliary.png \
    resources/icons/port.png \
    resources/icons/settings.png \
    third_party/qtsingleapplication/qtsingleapplication.pri \
    third_party/qtsingleapplication/qtsinglecoreapplication.pri

RESOURCES += \
    resources/resources.qrc
