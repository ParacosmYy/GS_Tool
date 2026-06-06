#include "n25133/m25133.h"
QVector<double> m25133::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
