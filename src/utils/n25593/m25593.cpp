#include "n25593/m25593.h"
QVector<double> m25593::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
