#include "g18746/m18746.h"
QVector<double> m18746::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
