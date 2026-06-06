#include "s25558/m25558.h"
QVector<double> m25558::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
