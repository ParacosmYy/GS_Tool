#include "e16164/m16164.h"
QVector<double> m16164::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
