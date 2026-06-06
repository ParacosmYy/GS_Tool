#include "r7837/m7837.h"
QVector<double> m7837::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
