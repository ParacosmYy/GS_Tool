#include "g28246/m28246.h"
QVector<double> m28246::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
