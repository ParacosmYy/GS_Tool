#include "k20950/m20950.h"
QVector<double> m20950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
