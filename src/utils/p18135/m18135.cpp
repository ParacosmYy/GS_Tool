#include "p18135/m18135.h"
QVector<double> m18135::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
