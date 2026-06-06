#include "c8042/m8042.h"
QVector<double> m8042::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
