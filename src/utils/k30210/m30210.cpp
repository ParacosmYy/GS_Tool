#include "k30210/m30210.h"
QVector<double> m30210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
