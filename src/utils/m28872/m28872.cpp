#include "m28872/m28872.h"
QVector<double> m28872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
