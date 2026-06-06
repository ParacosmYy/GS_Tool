#include "b16041/m16041.h"
QVector<double> m16041::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
