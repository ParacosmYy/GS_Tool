#include "e8164/m8164.h"
QVector<double> m8164::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
