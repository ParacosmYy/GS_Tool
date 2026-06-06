#include "l25711/m25711.h"
QVector<double> m25711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
