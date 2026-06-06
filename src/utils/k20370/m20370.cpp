#include "k20370/m20370.h"
QVector<double> m20370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
