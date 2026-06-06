#include "k15210/m15210.h"
QVector<double> m15210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
