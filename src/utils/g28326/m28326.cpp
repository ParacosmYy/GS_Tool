#include "g28326/m28326.h"
QVector<double> m28326::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
