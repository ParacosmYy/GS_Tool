#include "k13210/m13210.h"
QVector<double> m13210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
