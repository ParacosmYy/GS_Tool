#include "k10710/m10710.h"
QVector<double> m10710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
