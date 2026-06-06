#include "k10530/m10530.h"
QVector<double> m10530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
