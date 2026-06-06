#include "l16711/m16711.h"
QVector<double> m16711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
