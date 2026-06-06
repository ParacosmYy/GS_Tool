#include "l12711/m12711.h"
QVector<double> m12711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
