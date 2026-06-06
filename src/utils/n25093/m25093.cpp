#include "n25093/m25093.h"
QVector<double> m25093::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
