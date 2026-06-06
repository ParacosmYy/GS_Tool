#include "m15912/m15912.h"
QVector<double> m15912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
