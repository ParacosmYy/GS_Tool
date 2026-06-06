#include "k26110/m26110.h"
QVector<double> m26110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
