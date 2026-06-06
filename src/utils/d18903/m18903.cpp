#include "d18903/m18903.h"
QVector<double> m18903::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
