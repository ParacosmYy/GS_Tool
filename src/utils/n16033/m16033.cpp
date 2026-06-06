#include "n16033/m16033.h"
QVector<double> m16033::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
