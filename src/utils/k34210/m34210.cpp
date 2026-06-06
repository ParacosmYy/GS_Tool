#include "k34210/m34210.h"
QVector<double> m34210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
