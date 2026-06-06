#include "s18138/m18138.h"
QVector<double> m18138::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
