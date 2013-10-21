TEMPLATE        = lib
TARGET          = DDIManager

DEFINES += DDIMANAGER_LIBRARY

include(../../../plugins/fmf_plugins.pri)
include($${PWD}/ddiplugin_dependencies.pri)

INCLUDEPATH += ../
DEPENDPATH += ../

HEADERS += \
    constants.h \
    ddiplugin.h \
    ddicore.h \
    database/ddidatabase.h \
    atc/atctablemodel.h \
    atc/atctreeproxymodel.h \
    atc/atccollectioneditorwidget.h \
    atc/atceditorwidget.h \
    atc/searchatcindatabasedialog.h \
    atc/atcmode.h \
    interactors/druginteractor.h \
    interactors/druginteractortablemodel.h \
    interactors/druginteractortreeproxymodel.h \
    interactors/interactoreditorwidget.h \
    interactors/interactormode.h \
    interactors/interactorselectordialog.h \
    interactors/druginteractorsortfilterproxymodel.h \
    ddi/drugdruginteractionmode.h \
    ddi/drugdruginteraction.h \
    ddi/drugdruginteractiontablemodel.h \
    ddi/drugdruginteractioneditorwidget.h \
    ddi/drugdruginteractionsortfilterproxymodel.h \
    routes/routesmodel.h \


SOURCES += \
    ddiplugin.cpp \
    ddicore.cpp \
    database/ddidatabase.cpp \
    atc/atctablemodel.cpp \
    atc/atctreeproxymodel.cpp \
    atc/atccollectioneditorwidget.cpp \
    atc/atceditorwidget.cpp \
    atc/searchatcindatabasedialog.cpp \
    atc/atcmode.cpp \
    interactors/druginteractor.cpp \
    interactors/druginteractortablemodel.cpp \
    interactors/druginteractortreeproxymodel.cpp \
    interactors/interactoreditorwidget.cpp \
    interactors/interactormode.cpp \
    interactors/interactorselectordialog.cpp \
    interactors/druginteractorsortfilterproxymodel.cpp \
    ddi/drugdruginteractionmode.cpp \
    ddi/drugdruginteraction.cpp \
    ddi/drugdruginteractiontablemodel.cpp \
    ddi/drugdruginteractioneditorwidget.cpp \
    ddi/drugdruginteractionsortfilterproxymodel.cpp \
    routes/routesmodel.cpp \


FORMS += \
    atc/atccollectioneditorwidget.ui \
    atc/atceditorwidget.ui \
    atc/searchatcindatabasedialog.ui \
    interactors/interactoreditorwidget.ui \
    interactors/interactorselectorwidget.ui \
    ddi/drugdruginteractioneditorwidget.ui \


OTHER_FILES += DDIManager.pluginspec

equals(TEST, 1) {
    SOURCES += \
        tests/test_druginteractor.cpp
}

# include translations
TRANSLATION_NAME = ddimanager
include($${SOURCES_ROOT_PATH}/buildspecs/translations.pri)
