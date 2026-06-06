#include "p9915/m9915.h"
QVector<double> m9915::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
