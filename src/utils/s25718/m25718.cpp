#include "s25718/m25718.h"
QVector<double> m25718::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
