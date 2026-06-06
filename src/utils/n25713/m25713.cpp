#include "n25713/m25713.h"
QVector<double> m25713::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
