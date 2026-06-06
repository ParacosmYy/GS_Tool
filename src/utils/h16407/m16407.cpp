#include "h16407/m16407.h"
QVector<double> m16407::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
