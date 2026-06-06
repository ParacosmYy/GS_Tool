#include "k11210/m11210.h"
QVector<double> m11210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
