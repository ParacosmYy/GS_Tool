#include "b25041/m25041.h"
QVector<double> m25041::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
