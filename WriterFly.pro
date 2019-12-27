#-------------------------------------------------
#
# Project created by QtCreator 2018-08-05T10:57:59
#
#-------------------------------------------------

QT       += core gui network axcontainer
RC_ICONS = appicon.ico
#RC_FILE += version.rc

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

if (contains(DEFINES,Q_OS_WIN))
{
    Qt += axcontainer
}

# PRECOMPILED_HEADER = stable.h # 使用预编译头加快编译速度
# QMAKE_CXXFLAGS += /MP         # 使用并行编译

TARGET = WriterFly
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++11
#QMAKE_LFLAGS += /MANIFESTUAC:"level='requireAdministrator'uiAccess='false'"

#DEFINES += FRISO # 是否使用Friso分词工具（到VS上Friso库会导致无法编译，只能放弃）
#DEFINES += NET_DEBUG
#DEFINES += TEXT_ANALYZE_MODE2

INCLUDEPATH += dirlayout\
    dirlayout/dir_page\
    dirlayout/dir_page/dirsettings\
    dirlayout/outline_page\
    dirlayout/cardlib_page\
    editor\
    editor/general_io\
    editor/chapter_ai\
    editor/animation\
    editor/interaction\
    editor/search\
    editor/board\
    editor/motion\
    mainwindow\
    mainwindow/notification\
    mainwindow/shortcut\
    mainwindow/theme\
    settings\
    settings/settings_old\
    settings/settings_dialogs\
    settings/settings_global\
    settings/settings_window\
    nlp\
    utils\
    utils/cryption\
    utils/base\
    utils/CPU_ID\
    utils/segment\
    utils/ai_analyze\
    utils/external\
    widgets\
    widgets/animations\
    widgets/events\
    widgets/transitions\
    widgets/titlebar\
    widgets/beauty\
    widgets/win_buttons\
    widgets/dynamic_background\
    widgets/escape_dialog\
    widgets/splash\
    widgets/dialogs\
    im_ex_port\
    threads\
    globals\
    network\
    search\
    tools\
    tools/jieba/include\
    tools/jieba/deps\
    cloud\
    cloud/sequeue\
    darkroom\
    cardlib\


HEADERS  += mainwindow/mainwindow.h \
    cardlib/cardbean.h \
    cardlib/cardlibloadthread.h \
    cardlib/selecteditingcardsthread.h \
    dirlayout/cardlib_page/cardcardwidget.h \
    cardlib/cardeditor.h \
    cardlib/cardlib.h \
    dirlayout/cardlib_page/cardlistwidget.h \
    cardlib/subjectcolordialog.h \
    cardlib/subjectcolorseries.h \
    dirlayout/cardlib_page/cardlibgroup.h \
    cardlib/cardlibmanager.h \
    cardlib/cardlibhighlight.h \
    editor/motion/motionboard.h \
    editor/motion/motioncontentnumber.h \
    mainwindow/shortcut/shortcutentry.h \
    mainwindow/shortcut/shortcutinfoeditor.h \
    mainwindow/shortcut/shortcutkeymanager.h \
    mainwindow/shortcut/shortcutwindow.h \
    mainwindow/theme/thememanagewindow.h \
    nlp/cardlibtagsai.h \
    nlp/locatemutiple.h \
    nlp/predictbooks.h \
    settings/settings_global/settingbean.h \
    settings/settings_global/settingsmanager.h \
    settings/settings_window/usersettingitem2.h \
    settings/settings_window/usersettingswindow2.h \
    utils/external/wordutil.h \
    widgets/dialogs/bordershadow.h \
    widgets/dialogs/colordialog.h \
    widgets/splash/splashscreen.h \
    widgets/titlebar/customtitlebar.h \
    widgets/titlebar/customtitlebarwindow.h \
    widgets/titlebar/mytitlebar.h \
    mainwindow/basewindow.h \
    globals/defines.h \
    dirlayout/dir_page/chapterinfo.h \
    dirlayout/dir_page/noveldirmodel.h \
    dirlayout/dir_page/noveldiritem.h \
    dirlayout/dir_page/dirsettings/noveldirsettings.h \
    globals/globalvar.h \
    dirlayout/dir_page/noveldirmdata.h \
    dirlayout/dir_page/noveldirgroup.h \
    dirlayout/dir_page/noveldirlistview.h \
    settings/settings_global/settings.h \
    utils/base/stringutil.h \
    editor/chapter_ai/novelai.h \
    settings/settings_global/usettings.h \
    utils/base/fileutil.h \
    dirlayout/dir_page/noveldirdelegate.h \
    editor/chapter_ai/novelaibase.h \
    editor/stackwidget.h \
    widgets/transitions/stackwidgetanimation.h \
    editor/interaction/editorrecorderitem.h \
    utils/segment/frisoutil.h \
    utils/base/pinyinutil.h \
    widgets/events/splitterwidget.h \
    widgets/events/dragsizewidget.h \
    settings/settings_old/usersettingswindow.h \
    settings/settings_old/usersettingsitem.h \
    widgets/titlebar/anivlabel.h \
    widgets/animations/aniswitch.h \
    widgets/animations/anifbutton.h \
    widgets/animations/aninlabel.h \
    mainwindow/transparentcontainer.h \
    utils/ai_analyze/lexicon.h \
    globals/runtimeinfo.h \
    widgets/transitions/zoomgeometryanimationwidget.h \
    mainwindow/novelschedulewidget.h \
    widgets/events/mylabel.h \
    im_ex_port/im_ex_window.h \
    im_ex_port/exportpage.h \
    im_ex_port/importpage.h \
    im_ex_port/importmzfypage.h \
    dirlayout/dir_page/dirsettings/dirsettingswidget.h \
    globals/stable.h \
    editor/interaction/selectionitem.h \
    editor/interaction/selectionitemlist.h \
    threads/applicationupdatethread.h \
    network/ \
    network/filedownloadmanager.h \
    utils/base/netutil.h \
    editor/animation/editorcursor.h \
    editor/animation/editorinputmanager.h \
    editor/animation/editorinputcharactor.h \
    widgets/beauty/circlebutton.h \
    widgets/animations/anicirclelabel.h \
    widgets/animations/aninumberlabel.h \
    widgets/beauty/circlefloatbutton.h \
    widgets/beauty/generalbuttoninterface.h \
    dirlayout/sidebarwidget.h \
    settings/settings_dialogs/homonymcoverdata.h \
    dirlayout/outline_page/outlinegroup.h \
    dirlayout/outline_page/outlinesettings.h \
    editor/chapter_ai/chaptereditor.h \
    editor/chapter_ai/editinginfo.h \
    settings/settings_dialogs/homonymcoverwindow.h \
    globals/apptheme.h \
    widgets/animations/bezierwavebean.h \
    editor/search/editorsearchwidget.h \
    search/searchdelegate.h \
    search/searchresultbean.h \
    search/searchmodel.h \
    search/searchpanel.h \
    widgets/beauty/generallistview.h \
    widgets/beauty/generalcombobox.h \
    editor/interaction/editorscrollbean.h \
    editor/chapter_ai/myhightlighter.h \
    editor/general_io/generaleditor.h \
    utils/ai_analyze/noveltools.h \
    editor/board/boardbase.h \
    editor/board/boardmanager.h \
    editor/board/boarditem.h \
    globals/globaldata.h \
    editor/board/boardwidget.h \
    widgets/animations/anitabwidget.h \
    editor/board/boardpage.h \
    widgets/events/messagebox.h \
    widgets/beauty/squarebutton.h \
    utils/cryption/aes.h \
    utils/cryption/aesutil.h \
    cloud/useraccount.h \
    widgets/win_buttons/interactivebuttonbase.h \
    widgets/win_buttons/waterfallbuttongroup.h \
    widgets/win_buttons/winclosebutton.h \
    widgets/win_buttons/winmaxbutton.h \
    widgets/win_buttons/winminbutton.h \
    widgets/win_buttons/winmenubutton.h \
    widgets/win_buttons/winrestorebutton.h \
    widgets/win_buttons/watercirclebutton.h \
    widgets/win_buttons/pointmenubutton.h \
    widgets/win_buttons/threedimenbutton.h \
    utils/cryption/bxorcryptutil.h \
    editor/animation/smoothscrollbean.h \
    widgets/win_buttons/waterfloatbutton.h \
    widgets/win_buttons/waterzoombutton.h \
    widgets/dynamic_background/dynamicbackgroundinterface.h \
    widgets/dynamic_background/dynamicbackgroundpure.h \
    widgets/dynamic_background/dynamicbackgroundgradient.h \
    settings/settings_dialogs/dynamicbackgroundsettings.h \
    settings/settings_dialogs/dynamicbackgroundshowwidget.h \
    mainwindow/mainmenuwidget.h \
    settings/settings_dialogs/dynamicbackgroundpreviewwidget.h \
    utils/segment/jiebautil.h \
    utils/segment/segmenttool.h \
    utils/ai_analyze/warmwishutil.h \
    utils/CPU_ID/cpu_id_util.h \
    utils/CPU_ID/system_cpuid.h \
    nlp/emotionfilter.h \
    nlp/environmentpictures.h \
    cloud/syncthread.h \
    cloud/loginwindow.h \
    widgets/dialogs/mydialog.h \
    cloud/syncstatewindow.h \
    cloud/sequeue/downloaddelegate.h \
    cloud/sequeue/syncsequeuemodel.h \
    cloud/sequeue/syncfinishedqueuemodel.h \
    cloud/sequeue/syncfinishedsequeuedelegate.h \
    dirlayout/outline_page/treeoutlinewindow.h \
    widgets/beauty/blurwidget.h \
    darkroom/darkroomwidget.h \
    darkroom/darkroom.h \
    darkroom/digitaltimestamppredict.h \
    widgets/events/myspinbox.h \
    widgets/win_buttons/winsidebarbutton.h \
    widgets/events/inputdialog.h \
    search/searchlistwidget.h \
    widgets/escape_dialog/hoverbutton.h \
    widgets/escape_dialog/escapedialog.h \
    mainwindow/notification/notificationentry.h \
    mainwindow/notification/tipbox.h \
    mainwindow/notification/tipcard.h

SOURCES += main.cpp\
    cardlib/cardlibloadthread.cpp \
    cardlib/selecteditingcardsthread.cpp \
    dirlayout/cardlib_page/cardcardwidget.cpp \
    cardlib/cardeditor.cpp \
    cardlib/cardlib.cpp \
    dirlayout/cardlib_page/cardlistwidget.cpp \
    cardlib/subjectcolordialog.cpp \
    cardlib/subjectcolorseries.cpp \
    dirlayout/cardlib_page/cardlibgroup.cpp \
    cardlib/cardlibmanager.cpp \
    editor/motion/motionboard.cpp \
    editor/motion/motioncontentnumber.cpp \
    mainwindow/mainwindow.cpp \
    mainwindow/shortcut/shortcutinfoeditor.cpp \
    mainwindow/shortcut/shortcutkeymanager.cpp \
    mainwindow/shortcut/shortcutwindow.cpp \
    mainwindow/theme/thememanagewindow.cpp \
    nlp/locatemutiple.cpp \
    nlp/predictbooks.cpp \
    settings/settings_global/settingsmanager.cpp \
    settings/settings_global/settingsmanager_items.cpp \
    settings/settings_window/usersettingswindow2.cpp \
    utils/external/wordutil.cpp \
    widgets/dialogs/bordershadow.cpp \
    widgets/dialogs/colordialog.cpp \
    widgets/splash/splashscreen.cpp \
    widgets/titlebar/customtitlebar.cpp \
    widgets/titlebar/customtitlebarwindow.cpp \
    widgets/titlebar/mytitlebar.cpp \
    mainwindow/basewindow.cpp \
    dirlayout/dir_page/chapterinfo.cpp \
    dirlayout/dir_page/noveldiritem.cpp \
    dirlayout/dir_page/dirsettings/noveldirsettings.cpp \
    dirlayout/dir_page/noveldirmdata.cpp \
    dirlayout/dir_page/noveldirgroup.cpp \
    dirlayout/dir_page/noveldirlistview.cpp \
    globals/globalvar.cpp \
    utils/base/stringutil.cpp \
    editor/chapter_ai/novelai.cpp \
    utils/base/fileutil.cpp \
    editor/chapter_ai/novelaibase.cpp \
    editor/stackwidget.cpp \
    widgets/transitions/stackwidgetanimation.cpp \
    settings/settings_global/usettings.cpp \
    editor/interaction/editorrecorderitem.cpp \
    utils/segment/frisoutil.cpp \
    widgets/events/splitterwidget.cpp \
    widgets/events/dragsizewidget.cpp \
    settings/settings_old/usersettingswindow.cpp \
    widgets/animations/aniswitch.cpp \
    settings/settings_old/usersettingsitem.cpp \
    widgets/titlebar/anivlabel.cpp \
    widgets/animations/anifbutton.cpp \
    widgets/animations/aninlabel.cpp \
    mainwindow/transparentcontainer.cpp \
    utils/ai_analyze/lexicon.cpp \
    widgets/transitions/zoomgeometryanimationwidget.cpp \
    mainwindow/novelschedulewidget.cpp \
    widgets/events/mylabel.cpp \
    im_ex_port/im_ex_window.cpp \
    im_ex_port/exportpage.cpp \
    im_ex_port/importpage.cpp \
    im_ex_port/importmzfypage.cpp \
    dirlayout/dir_page/dirsettings/dirsettingswidget.cpp \
    globals/runtimeinfo.cpp \
    editor/interaction/selectionitemlist.cpp \
    network/filedownloadmanager.cpp \
    editor/animation/editorcursor.cpp \
    editor/animation/editorinputmanager.cpp \
    editor/animation/editorinputcharactor.cpp \
    widgets/beauty/circlebutton.cpp \
    widgets/animations/anicirclelabel.cpp \
    widgets/animations/aninumberlabel.cpp \
    widgets/beauty/circlefloatbutton.cpp \
    widgets/beauty/generalbuttoninterface.cpp \
    dirlayout/sidebarwidget.cpp \
    dirlayout/outline_page/outlinegroup.cpp \
    editor/chapter_ai/chaptereditor.cpp \
    editor/chapter_ai/editinginfo.cpp \
    settings/settings_dialogs/homonymcoverwindow.cpp \
    globals/apptheme.cpp \
    widgets/animations/bezierwavebean.cpp \
    editor/search/editorsearchwidget.cpp \
    search/searchpanel.cpp \
    widgets/beauty/generallistview.cpp \
    widgets/beauty/generalcombobox.cpp \
    editor/chapter_ai/myhightlighter.cpp \
    editor/general_io/generaleditor.cpp \
    editor/board/boardbase.cpp \
    editor/board/boardmanager.cpp \
    editor/board/boardwidget.cpp \
    widgets/animations/anitabwidget.cpp \
    editor/board/boardpage.cpp \
    widgets/events/messagebox.cpp \
    widgets/beauty/squarebutton.cpp \
    utils/cryption/aes.cpp \
    cloud/useraccount.cpp \
    widgets/win_buttons/interactivebuttonbase.cpp \
    widgets/win_buttons/waterfallbuttongroup.cpp \
    widgets/win_buttons/winclosebutton.cpp \
    widgets/win_buttons/winmaxbutton.cpp \
    widgets/win_buttons/winminbutton.cpp \
    widgets/win_buttons/winmenubutton.cpp \
    widgets/win_buttons/winrestorebutton.cpp \
    widgets/win_buttons/watercirclebutton.cpp \
    widgets/win_buttons/waterfloatbutton.cpp \
    widgets/win_buttons/waterzoombutton.cpp \
    widgets/dynamic_background/dynamicbackgroundinterface.cpp \
    widgets/dynamic_background/dynamicbackgroundpure.cpp \
    widgets/dynamic_background/dynamicbackgroundgradient.cpp \
    settings/settings_dialogs/dynamicbackgroundsettings.cpp \
    mainwindow/mainmenuwidget.cpp \
    utils/segment/jiebautil.cpp \
    utils/ai_analyze/warmwishtutil.cpp \
    utils/CPU_ID/system_cpuid.cpp \
    nlp/emotionfilter.cpp \
    nlp/environmentpictures.cpp \
    cloud/syncthread.cpp \
    cloud/loginwindow.cpp \
    widgets/dialogs/mydialog.cpp \
    cloud/syncstatewindow.cpp \
    dirlayout/outline_page/treeoutlinewindow.cpp \
    widgets/beauty/blurwidget.cpp \
    darkroom/darkroomwidget.cpp \
    darkroom/darkroom.cpp \
    widgets/events/myspinbox.cpp \
    widgets/win_buttons/winsidebarbutton.cpp \
    widgets/win_buttons/pointmenubutton.cpp \
    widgets/win_buttons/threedimenbutton.cpp \
    widgets/events/inputdialog.cpp \
    mainwindow/basewindowmenu.cpp \
    widgets/escape_dialog/hoverbutton.cpp \
    widgets/escape_dialog/escapedialog.cpp \
    mainwindow/notification/tipbox.cpp \
    mainwindow/notification/tipcard.cpp

DISTFILES += \
    CHANGELOG.md \
    image/close.png \
    image/max.png \
    image/min.png \
    image/sure.png \
    image/appicon.png \
    stylesheet/CustomTitleBar.css \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    docs/README.md \
    docs/UPDATE.md \
    README.md \
    resources/documents/所有设置项说明.xlsx

RESOURCES += \
    resources.qrc

#if (contains(DEFINES,FRISO))
#{
#    win32:CONFIG(release, debug|release): LIBS += -L$$PWD/tools/friso/ -lFrisoLib
#    else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/tools/friso/ -lFrisoLib
#    else:unix: #LIBS += -L$$PWD/./ -lFrisoLib
#}


ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

