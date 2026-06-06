#include "n25673/m25673.h"
QVector<double> m25673::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
