#include "m15112/m15112.h"
QVector<double> m15112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
