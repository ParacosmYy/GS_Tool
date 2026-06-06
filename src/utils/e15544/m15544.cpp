#include "e15544/m15544.h"
QVector<double> m15544::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
