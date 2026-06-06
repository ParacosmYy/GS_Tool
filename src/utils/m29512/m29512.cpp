#include "m29512/m29512.h"
QVector<double> m29512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
