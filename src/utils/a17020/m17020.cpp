#include "a17020/m17020.h"
QVector<double> m17020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
