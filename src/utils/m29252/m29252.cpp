#include "m29252/m29252.h"
QVector<double> m29252::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
