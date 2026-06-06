#include "k15950/m15950.h"
QVector<double> m15950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
