#include "m15372/m15372.h"
QVector<double> m15372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
