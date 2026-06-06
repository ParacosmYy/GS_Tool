#include "g28646/m28646.h"
QVector<double> m28646::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
