#include "i8308/m8308.h"
QVector<double> m8308::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
