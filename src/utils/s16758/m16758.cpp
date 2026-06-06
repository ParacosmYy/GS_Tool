#include "s16758/m16758.h"
QVector<double> m16758::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
