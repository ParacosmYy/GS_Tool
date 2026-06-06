#include "a9020/m9020.h"
QVector<double> m9020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
