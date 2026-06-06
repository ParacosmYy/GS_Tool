#include "d9803/m9803.h"
QVector<double> m9803::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
