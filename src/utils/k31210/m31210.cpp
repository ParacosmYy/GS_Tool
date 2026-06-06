#include "k31210/m31210.h"
QVector<double> m31210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
