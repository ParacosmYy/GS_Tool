#include "t25119/m25119.h"
QVector<double> m25119::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
