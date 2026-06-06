#include "m8972/m8972.h"
QVector<double> m8972::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
