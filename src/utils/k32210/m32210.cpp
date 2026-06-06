#include "k32210/m32210.h"
QVector<double> m32210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
