#include "a21280/m21280.h"
QVector<double> m21280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
