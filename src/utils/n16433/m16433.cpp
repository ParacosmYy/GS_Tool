#include "n16433/m16433.h"
QVector<double> m16433::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
