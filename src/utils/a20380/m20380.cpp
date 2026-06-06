#include "a20380/m20380.h"
QVector<double> m20380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
