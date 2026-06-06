#include "m16052/m16052.h"
QVector<double> m16052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
