#include "m36792/m36792.h"
QVector<double> m36792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
