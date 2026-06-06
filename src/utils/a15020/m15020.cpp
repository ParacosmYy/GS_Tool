#include "a15020/m15020.h"
QVector<double> m15020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
