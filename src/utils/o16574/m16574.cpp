#include "o16574/m16574.h"
QVector<double> m16574::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
