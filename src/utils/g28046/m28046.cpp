#include "g28046/m28046.h"
QVector<double> m28046::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
