#include "s28818/m28818.h"
QVector<double> m28818::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
