#include "c16502/m16502.h"
QVector<double> m16502::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
