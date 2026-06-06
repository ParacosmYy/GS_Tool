#include "n25433/m25433.h"
QVector<double> m25433::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
