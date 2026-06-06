#include "p25915/m25915.h"
QVector<double> m25915::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
