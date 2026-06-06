#include "a15780/m15780.h"
QVector<double> m15780::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
