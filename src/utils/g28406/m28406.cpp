#include "g28406/m28406.h"
QVector<double> m28406::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
