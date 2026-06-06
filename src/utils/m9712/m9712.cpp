#include "m9712/m9712.h"
QVector<double> m9712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
