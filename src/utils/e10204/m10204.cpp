#include "e10204/m10204.h"
QVector<double> m10204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
