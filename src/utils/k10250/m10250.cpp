#include "k10250/m10250.h"
QVector<double> m10250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
