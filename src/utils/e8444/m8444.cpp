#include "e8444/m8444.h"
QVector<double> m8444::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
