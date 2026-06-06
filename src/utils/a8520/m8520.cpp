#include "a8520/m8520.h"
QVector<double> m8520::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
