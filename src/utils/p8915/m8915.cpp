#include "p8915/m8915.h"
QVector<double> m8915::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
