#include "n25833/m25833.h"
QVector<double> m25833::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
