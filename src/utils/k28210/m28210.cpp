#include "k28210/m28210.h"
QVector<double> m28210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
