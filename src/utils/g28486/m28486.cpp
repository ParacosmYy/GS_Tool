#include "g28486/m28486.h"
QVector<double> m28486::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
