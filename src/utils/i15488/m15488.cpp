#include "i15488/m15488.h"
QVector<double> m15488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
