#include "q8056/m8056.h"
QVector<double> m8056::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
