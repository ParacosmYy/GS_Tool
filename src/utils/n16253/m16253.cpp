#include "n16253/m16253.h"
QVector<double> m16253::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
