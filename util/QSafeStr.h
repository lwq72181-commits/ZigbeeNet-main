#ifndef QSAFESTR_H
#define QSAFESTR_H

#include "TSafeStr.h"
#include <QString>

namespace comn
{

template < class T >
int copyStr(T& t, const QString& strSource)
{
    std::string src = strSource.toStdString();
    return copyStr(t, sizeof(t), src.c_str(), src.length());
}


}


#endif // QSAFESTR_H
