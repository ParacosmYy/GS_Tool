#include "g21326/m21326.h"
QVector<double> m21326::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
