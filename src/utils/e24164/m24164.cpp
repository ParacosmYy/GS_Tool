#include "e24164/m24164.h"
QVector<double> m24164::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
