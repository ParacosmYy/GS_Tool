#include "s16818/m16818.h"
QVector<double> m16818::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
