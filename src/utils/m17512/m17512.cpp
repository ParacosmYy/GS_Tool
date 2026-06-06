#include "m17512/m17512.h"
QVector<double> m17512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
