#include "o16314/m16314.h"
QVector<double> m16314::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
