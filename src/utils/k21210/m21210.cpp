#include "k21210/m21210.h"
QVector<double> m21210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
