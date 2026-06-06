#include "p8095/m8095.h"
QVector<double> m8095::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
