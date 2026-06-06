#include "m12912/m12912.h"
QVector<double> m12912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
