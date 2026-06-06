#include "k24630/m24630.h"
QVector<double> m24630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
