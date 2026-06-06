#include "h16067/m16067.h"
QVector<double> m16067::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
