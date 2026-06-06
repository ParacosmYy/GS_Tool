#include "m10552/m10552.h"
QVector<double> m10552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
