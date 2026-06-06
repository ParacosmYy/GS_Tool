#include "h16307/m16307.h"
QVector<double> m16307::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
