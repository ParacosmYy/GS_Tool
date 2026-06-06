#include "k18210/m18210.h"
QVector<double> m18210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
