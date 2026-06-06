#include "g29066/m29066.h"
QVector<double> m29066::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
