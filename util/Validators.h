#ifndef VALIDATORS_H
#define VALIDATORS_H

#include <QValidator>

class Validators
{
public:
    Validators();

    static QValidator* port();
    static QValidator* ip();
    static QValidator* name(int minLength, int maxLength);
    static QValidator* digit(int minLength, int maxLength);

    static QValidator* number(int minValue, int maxValue);
    static QValidator* user(int minLength, int maxLength);

};

#endif // VALIDATORS_H
