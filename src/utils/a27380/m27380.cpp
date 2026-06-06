#include "a27380/m27380.h"
QVector<double> m27380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
