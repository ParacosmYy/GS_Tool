#include "b16381/m16381.h"
QVector<double> m16381::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
