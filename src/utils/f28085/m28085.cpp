#include "f28085/m28085.h"
QVector<double> m28085::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
