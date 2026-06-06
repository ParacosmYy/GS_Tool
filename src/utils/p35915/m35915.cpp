#include "p35915/m35915.h"
QVector<double> m35915::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
