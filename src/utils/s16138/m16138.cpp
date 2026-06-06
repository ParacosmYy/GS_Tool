#include "s16138/m16138.h"
QVector<double> m16138::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
