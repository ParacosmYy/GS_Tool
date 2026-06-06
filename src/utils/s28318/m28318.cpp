#include "s28318/m28318.h"
QVector<double> m28318::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
