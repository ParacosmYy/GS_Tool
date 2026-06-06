#include "d25803/m25803.h"
QVector<double> m25803::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
