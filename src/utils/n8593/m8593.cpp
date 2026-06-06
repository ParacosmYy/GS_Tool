#include "n8593/m8593.h"
QVector<double> m8593::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
