#include "n25873/m25873.h"
QVector<double> m25873::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
