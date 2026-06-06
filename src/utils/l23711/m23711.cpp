#include "l23711/m23711.h"
QVector<double> m23711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
