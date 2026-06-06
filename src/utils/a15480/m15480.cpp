#include "a15480/m15480.h"
QVector<double> m15480::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
