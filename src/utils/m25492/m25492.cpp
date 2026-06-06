#include "m25492/m25492.h"
QVector<double> m25492::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
