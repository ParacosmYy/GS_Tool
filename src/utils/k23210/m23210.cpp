#include "k23210/m23210.h"
QVector<double> m23210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
