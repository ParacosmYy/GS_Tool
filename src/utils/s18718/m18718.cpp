#include "s18718/m18718.h"
QVector<double> m18718::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
