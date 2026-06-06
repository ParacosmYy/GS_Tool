#include "g28926/m28926.h"
QVector<double> m28926::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
