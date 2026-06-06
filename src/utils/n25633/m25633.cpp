#include "n25633/m25633.h"
QVector<double> m25633::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
