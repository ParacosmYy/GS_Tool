#include "d25563/m25563.h"
QVector<double> m25563::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
