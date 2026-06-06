#include "b28501/m28501.h"
QVector<double> m28501::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
