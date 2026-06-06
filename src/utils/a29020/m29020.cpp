#include "a29020/m29020.h"
QVector<double> m29020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
