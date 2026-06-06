#include "m15672/m15672.h"
QVector<double> m15672::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
