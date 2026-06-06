#include "c28602/m28602.h"
QVector<double> m28602::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
