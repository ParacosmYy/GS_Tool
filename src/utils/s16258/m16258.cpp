#include "s16258/m16258.h"
QVector<double> m16258::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
