#include "s18318/m18318.h"
QVector<double> m18318::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
