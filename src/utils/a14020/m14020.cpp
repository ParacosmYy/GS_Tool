#include "a14020/m14020.h"
QVector<double> m14020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
