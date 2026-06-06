#include "l8051/m8051.h"
QVector<double> m8051::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
