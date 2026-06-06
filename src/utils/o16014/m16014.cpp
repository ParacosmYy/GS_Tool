#include "o16014/m16014.h"
QVector<double> m16014::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
