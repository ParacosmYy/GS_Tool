#include "e25204/m25204.h"
QVector<double> m25204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
