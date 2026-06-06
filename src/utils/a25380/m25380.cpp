#include "a25380/m25380.h"
QVector<double> m25380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
