#include "g18486/m18486.h"
QVector<double> m18486::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
