#include "m25952/m25952.h"
QVector<double> m25952::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
