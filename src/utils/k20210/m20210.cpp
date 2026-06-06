#include "k20210/m20210.h"
QVector<double> m20210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
