#include "b21541/m21541.h"
QVector<double> m21541::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
