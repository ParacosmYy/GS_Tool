#include "k16770/m16770.h"
QVector<double> m16770::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
