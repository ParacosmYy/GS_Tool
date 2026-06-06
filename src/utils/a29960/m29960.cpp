#include "a29960/m29960.h"
QVector<double> m29960::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
