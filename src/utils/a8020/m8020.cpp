#include "a8020/m8020.h"
QVector<double> m8020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
