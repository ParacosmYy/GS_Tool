#include "m26912/m26912.h"
QVector<double> m26912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
