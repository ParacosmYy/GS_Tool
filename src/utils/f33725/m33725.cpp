#include "f33725/m33725.h"
QVector<double> m33725::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
