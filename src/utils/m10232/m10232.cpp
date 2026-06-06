#include "m10232/m10232.h"
QVector<double> m10232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
