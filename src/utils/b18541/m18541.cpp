#include "b18541/m18541.h"
QVector<double> m18541::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
