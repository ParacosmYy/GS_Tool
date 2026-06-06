#include "m26232/m26232.h"
QVector<double> m26232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
