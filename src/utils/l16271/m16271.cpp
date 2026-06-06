#include "l16271/m16271.h"
QVector<double> m16271::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
