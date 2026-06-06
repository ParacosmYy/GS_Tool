#include "k18050/m18050.h"
QVector<double> m18050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
