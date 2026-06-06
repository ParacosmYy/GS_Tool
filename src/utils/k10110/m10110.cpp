#include "k10110/m10110.h"
QVector<double> m10110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
