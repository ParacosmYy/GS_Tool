#include "k9210/m9210.h"
QVector<double> m9210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
