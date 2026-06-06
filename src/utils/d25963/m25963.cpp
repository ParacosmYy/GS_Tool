#include "d25963/m25963.h"
QVector<double> m25963::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
