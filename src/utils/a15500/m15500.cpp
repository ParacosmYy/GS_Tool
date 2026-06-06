#include "a15500/m15500.h"
QVector<double> m15500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
