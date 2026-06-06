#include "m25912/m25912.h"
QVector<double> m25912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
