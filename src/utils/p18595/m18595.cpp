#include "p18595/m18595.h"
QVector<double> m18595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
