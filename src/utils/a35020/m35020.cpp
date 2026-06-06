#include "a35020/m35020.h"
QVector<double> m35020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
