#include "m16332/m16332.h"
QVector<double> m16332::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
