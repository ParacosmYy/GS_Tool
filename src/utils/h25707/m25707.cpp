#include "h25707/m25707.h"
QVector<double> m25707::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
