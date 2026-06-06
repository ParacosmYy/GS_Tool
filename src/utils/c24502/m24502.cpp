#include "c24502/m24502.h"
QVector<double> m24502::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
