#include "m24912/m24912.h"
QVector<double> m24912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
