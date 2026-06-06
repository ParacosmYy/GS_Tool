#include "s15858/m15858.h"
QVector<double> m15858::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
