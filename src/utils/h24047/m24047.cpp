#include "h24047/m24047.h"
QVector<double> m24047::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
