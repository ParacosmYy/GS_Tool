#include "m15712/m15712.h"
QVector<double> m15712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
