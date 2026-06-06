#include "b18301/m18301.h"
QVector<double> m18301::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
