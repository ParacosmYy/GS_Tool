#include "c18502/m18502.h"
QVector<double> m18502::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
