#include "c28502/m28502.h"
QVector<double> m28502::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
