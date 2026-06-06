#include "d25003/m25003.h"
QVector<double> m25003::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
