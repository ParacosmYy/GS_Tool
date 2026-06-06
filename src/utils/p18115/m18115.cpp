#include "p18115/m18115.h"
QVector<double> m18115::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
