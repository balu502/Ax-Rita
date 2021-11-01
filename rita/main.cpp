#define QAX_DUMPCPP_RITA_NOINLINES

#include "rita.h"
#include <QApplication>
#include <QAxFactory>
#include <QDir>


QT_USE_NAMESPACE

QAXFACTORY_BEGIN("{BD4D3B5E-C5D3-460C-8926-0FD9AA439303}", "{29DA2296-AEC9-4F5A-BCD9-2B7C173E6629}")
    QAXCLASS(RiTApplication)
    QAXCLASS(RiTUser)
    QAXCLASS(RiTEntryCache)
QAXFACTORY_END()


int main(int argc, char *argv[]){

    RiTApplication::setAppNameDomain();

    QApplication
            app(argc, argv);
            app.setQuitOnLastWindowClosed(false);

    QDir::setCurrent( app.applicationDirPath() );

    // started by COM - don't do anything
    if (QAxFactory::isServer())
        return app.exec();

    // started by user
    RiTApplication
        appobject(0);
        appobject.setObjectName("RITA_From_Application");


    QAxFactory::startServer();
    QAxFactory::registerActiveObject(&appobject);

    appobject.execute(false);

//    QObject::connect( qApp, SIGNAL(lastWindowClosed()),
//                &appobject,   SLOT(quit()));

    return 0;
}
