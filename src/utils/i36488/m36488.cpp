#include "i36488/m36488.h"
QVector<double> m36488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
