#include "m16472/m16472.h"
QVector<double> m16472::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
