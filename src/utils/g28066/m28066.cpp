#include "g28066/m28066.h"
QVector<double> m28066::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
