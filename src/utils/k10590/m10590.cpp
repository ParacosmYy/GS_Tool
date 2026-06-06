#include "k10590/m10590.h"
QVector<double> m10590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
