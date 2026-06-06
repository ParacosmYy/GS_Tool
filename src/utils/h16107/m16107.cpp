#include "h16107/m16107.h"
QVector<double> m16107::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
