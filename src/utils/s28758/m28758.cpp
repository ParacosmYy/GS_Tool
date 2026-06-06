#include "s28758/m28758.h"
QVector<double> m28758::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
