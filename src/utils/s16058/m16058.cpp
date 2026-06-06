#include "s16058/m16058.h"
QVector<double> m16058::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
