#include "a24020/m24020.h"
QVector<double> m24020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
