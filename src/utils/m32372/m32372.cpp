#include "m32372/m32372.h"
QVector<double> m32372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
