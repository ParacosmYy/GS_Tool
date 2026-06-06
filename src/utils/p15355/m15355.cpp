#include "p15355/m15355.h"
QVector<double> m15355::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
