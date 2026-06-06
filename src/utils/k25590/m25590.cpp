#include "k25590/m25590.h"
QVector<double> m25590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
