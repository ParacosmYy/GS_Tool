#include "d8803/m8803.h"
QVector<double> m8803::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
