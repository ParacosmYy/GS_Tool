#include "g8946/m8946.h"
QVector<double> m8946::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
