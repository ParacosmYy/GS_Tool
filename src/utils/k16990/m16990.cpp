#include "k16990/m16990.h"
QVector<double> m16990::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
