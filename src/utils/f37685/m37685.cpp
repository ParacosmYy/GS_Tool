#include "f37685/m37685.h"
QVector<double> m37685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
