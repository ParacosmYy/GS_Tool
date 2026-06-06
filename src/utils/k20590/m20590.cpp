#include "k20590/m20590.h"
QVector<double> m20590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
