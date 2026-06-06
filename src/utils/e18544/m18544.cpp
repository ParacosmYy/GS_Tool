#include "e18544/m18544.h"
QVector<double> m18544::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
