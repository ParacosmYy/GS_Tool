#include "m36712/m36712.h"
QVector<double> m36712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
