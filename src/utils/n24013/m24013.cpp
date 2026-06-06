#include "n24013/m24013.h"
QVector<double> m24013::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
