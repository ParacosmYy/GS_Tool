#include "c15602/m15602.h"
QVector<double> m15602::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
