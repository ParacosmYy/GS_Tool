#include "b16161/m16161.h"
QVector<double> m16161::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
