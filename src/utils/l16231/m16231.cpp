#include "l16231/m16231.h"
QVector<double> m16231::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
