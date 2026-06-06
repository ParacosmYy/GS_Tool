#include "k33010/m33010.h"
QVector<double> m33010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
