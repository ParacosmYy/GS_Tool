#include "a15280/m15280.h"
QVector<double> m15280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
