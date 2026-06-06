#include "m8432/m8432.h"
QVector<double> m8432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
