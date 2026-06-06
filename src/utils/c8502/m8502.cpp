#include "c8502/m8502.h"
QVector<double> m8502::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
