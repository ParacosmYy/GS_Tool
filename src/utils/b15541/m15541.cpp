#include "b15541/m15541.h"
QVector<double> m15541::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
