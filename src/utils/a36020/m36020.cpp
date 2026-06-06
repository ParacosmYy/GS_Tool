#include "a36020/m36020.h"
QVector<double> m36020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
