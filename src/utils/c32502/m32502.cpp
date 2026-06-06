#include "c32502/m32502.h"
QVector<double> m32502::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
