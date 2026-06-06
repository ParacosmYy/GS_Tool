#include "l25551/m25551.h"
QVector<double> m25551::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
