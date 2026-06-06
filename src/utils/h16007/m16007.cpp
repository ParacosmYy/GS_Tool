#include "h16007/m16007.h"
QVector<double> m16007::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
