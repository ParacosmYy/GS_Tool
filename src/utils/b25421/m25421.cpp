#include "b25421/m25421.h"
QVector<double> m25421::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
