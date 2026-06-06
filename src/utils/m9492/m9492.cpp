#include "m9492/m9492.h"
QVector<double> m9492::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
