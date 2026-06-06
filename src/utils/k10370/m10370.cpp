#include "k10370/m10370.h"
QVector<double> m10370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
