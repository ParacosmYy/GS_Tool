#include "p25875/m25875.h"
QVector<double> m25875::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
