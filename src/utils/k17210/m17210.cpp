#include "k17210/m17210.h"
QVector<double> m17210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
