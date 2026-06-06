#include "b19541/m19541.h"
QVector<double> m19541::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
