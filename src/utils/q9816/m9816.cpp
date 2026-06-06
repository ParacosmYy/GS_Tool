#include "q9816/m9816.h"
QVector<double> m9816::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
