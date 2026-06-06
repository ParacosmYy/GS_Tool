#include "p15035/m15035.h"
QVector<double> m15035::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
