/*
#---------------------------------------------------
#
#   PRAGA GIS
#
#   this file is part of CRITERIA3D distribution
#
#---------------------------------------------------
*/


#include <QApplication>
#include <QtNetwork/QNetworkProxy>
#include <QMessageBox>

#include "mainWindow.h"

#ifdef GDAL
    #include <gdal.h>
#endif


bool setProxy(QString hostName, unsigned short port)
{
    QNetworkProxy myProxy;

    myProxy.setType(QNetworkProxy::HttpProxy);
    myProxy.setHostName(hostName);
    myProxy.setPort(port);

    try {
       QNetworkProxy::setApplicationProxy(myProxy);
    }
    catch (...) {
        QMessageBox::information(nullptr, "Error in proxy configuration!", "");
        return false;
    }

    return true;
}


int main(int argc, char *argv[])
{
    QApplication myApp(argc, argv);

    QNetworkProxyFactory::setUseSystemConfiguration(true);

    QApplication::setOverrideCursor(Qt::ArrowCursor);

    #ifdef GDAL
        GDALAllRegister();
    #endif

    MainWindow w;
    w.show();

    return myApp.exec();
}


