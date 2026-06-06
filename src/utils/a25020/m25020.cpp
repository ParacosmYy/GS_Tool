#include "a25020/m25020.h"
QVector<double> m25020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
