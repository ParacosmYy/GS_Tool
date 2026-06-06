#include "h25127/m25127.h"
QVector<double> m25127::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
