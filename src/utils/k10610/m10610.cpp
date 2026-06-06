#include "k10610/m10610.h"
QVector<double> m10610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
