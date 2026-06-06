#include "b14541/m14541.h"
QVector<double> m14541::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
