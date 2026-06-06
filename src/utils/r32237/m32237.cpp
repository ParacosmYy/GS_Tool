#include "r32237/m32237.h"
QVector<double> m32237::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
