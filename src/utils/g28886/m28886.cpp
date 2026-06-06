#include "g28886/m28886.h"
QVector<double> m28886::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
