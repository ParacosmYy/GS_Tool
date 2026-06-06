#include "g28266/m28266.h"
QVector<double> m28266::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
