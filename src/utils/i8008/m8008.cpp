#include "i8008/m8008.h"
QVector<double> m8008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
