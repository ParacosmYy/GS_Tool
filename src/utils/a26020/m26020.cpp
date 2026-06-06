#include "a26020/m26020.h"
QVector<double> m26020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
