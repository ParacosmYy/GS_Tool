#include "t26159/m26159.h"
QVector<double> m26159::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
