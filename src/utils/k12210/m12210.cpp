#include "k12210/m12210.h"
QVector<double> m12210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
