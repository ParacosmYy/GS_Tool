#include "m18492/m18492.h"
QVector<double> m18492::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
