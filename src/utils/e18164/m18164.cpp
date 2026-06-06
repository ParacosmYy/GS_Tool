#include "e18164/m18164.h"
QVector<double> m18164::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
