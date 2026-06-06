#include "p15055/m15055.h"
QVector<double> m15055::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
