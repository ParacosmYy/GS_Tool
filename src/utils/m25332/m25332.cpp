#include "m25332/m25332.h"
QVector<double> m25332::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
