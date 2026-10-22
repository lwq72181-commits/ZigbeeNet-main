#ifndef APPDIR_H
#define APPDIR_H

#include "BasicType.h"
#include <string>
#include <QString>

class AppDir
{
public:
    AppDir();

	static QString getAppDir();

	static QString getEtcDir();

	static QString getUploadDir();

	static QString getUsbDiskDir();

    static QString getLogDir();


};

#endif // APPDIR_H
