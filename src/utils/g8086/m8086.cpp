#include "g8086/m8086.h"
QVector<double> m8086::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
