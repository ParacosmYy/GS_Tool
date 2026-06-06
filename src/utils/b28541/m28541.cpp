#include "b28541/m28541.h"
QVector<double> m28541::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
