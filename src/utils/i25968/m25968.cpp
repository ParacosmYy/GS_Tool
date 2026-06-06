#include "i25968/m25968.h"
QVector<double> m25968::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
