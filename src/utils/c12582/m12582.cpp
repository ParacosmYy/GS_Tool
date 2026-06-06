#include "c12582/m12582.h"
QVector<double> m12582::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
