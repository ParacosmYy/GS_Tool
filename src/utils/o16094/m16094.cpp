#include "o16094/m16094.h"
QVector<double> m16094::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
