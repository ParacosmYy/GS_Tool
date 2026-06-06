#include "g29606/m29606.h"
QVector<double> m29606::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
