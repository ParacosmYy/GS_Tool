#include "s9118/m9118.h"
QVector<double> m9118::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
