#include "k27210/m27210.h"
QVector<double> m27210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
