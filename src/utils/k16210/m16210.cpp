#include "k16210/m16210.h"
QVector<double> m16210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
