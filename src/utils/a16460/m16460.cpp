#include "a16460/m16460.h"
QVector<double> m16460::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
