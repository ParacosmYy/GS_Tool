#include "k24110/m24110.h"
QVector<double> m24110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
