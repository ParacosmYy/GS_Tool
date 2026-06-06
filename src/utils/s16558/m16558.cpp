#include "s16558/m16558.h"
QVector<double> m16558::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
