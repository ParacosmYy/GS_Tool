#include "g28086/m28086.h"
QVector<double> m28086::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
