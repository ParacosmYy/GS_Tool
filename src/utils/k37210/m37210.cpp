#include "k37210/m37210.h"
QVector<double> m37210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
