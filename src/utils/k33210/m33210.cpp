#include "k33210/m33210.h"
QVector<double> m33210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
