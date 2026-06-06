#include "m9372/m9372.h"
QVector<double> m9372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
