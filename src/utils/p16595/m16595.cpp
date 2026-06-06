#include "p16595/m16595.h"
QVector<double> m16595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
