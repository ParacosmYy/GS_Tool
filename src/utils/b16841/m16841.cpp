#include "b16841/m16841.h"
QVector<double> m16841::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
