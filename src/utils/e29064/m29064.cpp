#include "e29064/m29064.h"
QVector<double> m29064::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
