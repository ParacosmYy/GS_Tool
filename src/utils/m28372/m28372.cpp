#include "m28372/m28372.h"
QVector<double> m28372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
