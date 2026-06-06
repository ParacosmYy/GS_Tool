#include "d19803/m19803.h"
QVector<double> m19803::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
