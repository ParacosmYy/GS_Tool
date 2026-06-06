#include "k16590/m16590.h"
QVector<double> m16590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
