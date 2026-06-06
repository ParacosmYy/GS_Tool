#include "m10412/m10412.h"
QVector<double> m10412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
