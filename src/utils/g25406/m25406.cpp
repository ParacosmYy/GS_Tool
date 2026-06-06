#include "g25406/m25406.h"
QVector<double> m25406::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
