#include "l9551/m9551.h"
QVector<double> m9551::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
