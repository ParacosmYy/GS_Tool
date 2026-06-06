#include "l33711/m33711.h"
QVector<double> m33711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
