#include "k25990/m25990.h"
QVector<double> m25990::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
