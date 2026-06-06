#include "p7915/m7915.h"
QVector<double> m7915::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
