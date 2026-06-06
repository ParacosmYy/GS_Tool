#include "b16681/m16681.h"
QVector<double> m16681::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
