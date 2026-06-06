#include "s25118/m25118.h"
QVector<double> m25118::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
