#include "m16432/m16432.h"
QVector<double> m16432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
