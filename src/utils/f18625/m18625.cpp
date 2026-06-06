#include "f18625/m18625.h"
QVector<double> m18625::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
