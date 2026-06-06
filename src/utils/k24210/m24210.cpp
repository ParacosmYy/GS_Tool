#include "k24210/m24210.h"
QVector<double> m24210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
