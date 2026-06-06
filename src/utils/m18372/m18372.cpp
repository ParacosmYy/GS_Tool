#include "m18372/m18372.h"
QVector<double> m18372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
