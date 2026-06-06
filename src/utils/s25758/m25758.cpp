#include "s25758/m25758.h"
QVector<double> m25758::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
