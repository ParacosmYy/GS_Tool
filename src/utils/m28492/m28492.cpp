#include "m28492/m28492.h"
QVector<double> m28492::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
