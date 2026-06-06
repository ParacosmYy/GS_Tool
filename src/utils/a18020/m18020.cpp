#include "a18020/m18020.h"
QVector<double> m18020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
