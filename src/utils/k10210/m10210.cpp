#include "k10210/m10210.h"
QVector<double> m10210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
