#include "m18872/m18872.h"
QVector<double> m18872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
