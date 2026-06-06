#include "b16281/m16281.h"
QVector<double> m16281::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
