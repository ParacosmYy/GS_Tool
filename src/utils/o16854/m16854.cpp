#include "o16854/m16854.h"
QVector<double> m16854::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
