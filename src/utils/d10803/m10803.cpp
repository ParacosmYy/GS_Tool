#include "d10803/m10803.h"
QVector<double> m10803::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
