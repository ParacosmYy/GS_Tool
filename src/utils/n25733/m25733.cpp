#include "n25733/m25733.h"
QVector<double> m25733::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
