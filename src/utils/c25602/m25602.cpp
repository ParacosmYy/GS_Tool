#include "c25602/m25602.h"
QVector<double> m25602::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
