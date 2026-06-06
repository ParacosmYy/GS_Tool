#include "g25926/m25926.h"
QVector<double> m25926::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
