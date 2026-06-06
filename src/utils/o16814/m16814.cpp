#include "o16814/m16814.h"
QVector<double> m16814::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
