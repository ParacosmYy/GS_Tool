#include "p24595/m24595.h"
QVector<double> m24595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
