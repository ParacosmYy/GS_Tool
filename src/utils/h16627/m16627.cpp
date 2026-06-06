#include "h16627/m16627.h"
QVector<double> m16627::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
