#include "j16509/m16509.h"
QVector<double> m16509::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
