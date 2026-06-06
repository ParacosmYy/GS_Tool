#include "h9887/m9887.h"
QVector<double> m9887::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
