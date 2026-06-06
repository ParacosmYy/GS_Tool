#include "k20630/m20630.h"
QVector<double> m20630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
