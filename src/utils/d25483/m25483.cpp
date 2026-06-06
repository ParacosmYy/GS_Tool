#include "d25483/m25483.h"
QVector<double> m25483::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
