#include "s25458/m25458.h"
QVector<double> m25458::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
