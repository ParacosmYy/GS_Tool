#include "m15752/m15752.h"
QVector<double> m15752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
