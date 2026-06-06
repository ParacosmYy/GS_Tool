#include "k10850/m10850.h"
QVector<double> m10850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
