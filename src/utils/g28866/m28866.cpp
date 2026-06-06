#include "g28866/m28866.h"
QVector<double> m28866::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
