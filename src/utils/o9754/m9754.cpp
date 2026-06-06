#include "o9754/m9754.h"
QVector<double> m9754::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
