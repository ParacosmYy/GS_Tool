#include "n25533/m25533.h"
QVector<double> m25533::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
