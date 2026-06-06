#include "o10754/m10754.h"
QVector<double> m10754::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
