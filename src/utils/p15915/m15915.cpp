#include "p15915/m15915.h"
QVector<double> m15915::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
