#include "k28950/m28950.h"
QVector<double> m28950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
