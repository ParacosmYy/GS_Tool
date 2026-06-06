#include "b8421/m8421.h"
QVector<double> m8421::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
