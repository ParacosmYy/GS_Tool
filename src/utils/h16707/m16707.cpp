#include "h16707/m16707.h"
QVector<double> m16707::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
