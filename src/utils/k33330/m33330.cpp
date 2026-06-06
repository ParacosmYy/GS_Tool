#include "k33330/m33330.h"
QVector<double> m33330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
