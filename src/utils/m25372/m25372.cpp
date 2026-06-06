#include "m25372/m25372.h"
QVector<double> m25372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
