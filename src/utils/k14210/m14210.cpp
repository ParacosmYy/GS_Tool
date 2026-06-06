#include "k14210/m14210.h"
QVector<double> m14210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
