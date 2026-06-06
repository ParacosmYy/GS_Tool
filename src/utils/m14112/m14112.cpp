#include "m14112/m14112.h"
QVector<double> m14112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
