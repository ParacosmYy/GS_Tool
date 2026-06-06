#include "m21372/m21372.h"
QVector<double> m21372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
