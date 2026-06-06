#include "o16254/m16254.h"
QVector<double> m16254::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
