#include "c9502/m9502.h"
QVector<double> m9502::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
