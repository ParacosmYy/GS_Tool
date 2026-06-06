#include "p18315/m18315.h"
QVector<double> m18315::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
