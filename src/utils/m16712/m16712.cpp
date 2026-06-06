#include "m16712/m16712.h"
QVector<double> m16712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
