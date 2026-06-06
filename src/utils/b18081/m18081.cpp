#include "b18081/m18081.h"
QVector<double> m18081::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
