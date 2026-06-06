#include "h16487/m16487.h"
QVector<double> m16487::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
