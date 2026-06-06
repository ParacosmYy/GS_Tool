#include "i17488/m17488.h"
QVector<double> m17488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
