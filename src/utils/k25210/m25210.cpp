#include "k25210/m25210.h"
QVector<double> m25210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
