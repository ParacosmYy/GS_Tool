#include "p18675/m18675.h"
QVector<double> m18675::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
