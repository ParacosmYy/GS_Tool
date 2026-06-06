#include "p18775/m18775.h"
QVector<double> m18775::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
