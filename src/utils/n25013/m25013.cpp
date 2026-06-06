#include "n25013/m25013.h"
QVector<double> m25013::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
