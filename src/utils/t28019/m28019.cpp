#include "t28019/m28019.h"
QVector<double> m28019::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
