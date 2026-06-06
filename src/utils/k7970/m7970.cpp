#include "k7970/m7970.h"
QVector<double> m7970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
