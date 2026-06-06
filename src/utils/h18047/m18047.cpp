#include "h18047/m18047.h"
QVector<double> m18047::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
