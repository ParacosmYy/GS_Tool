#include "g16066/m16066.h"
QVector<double> m16066::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
