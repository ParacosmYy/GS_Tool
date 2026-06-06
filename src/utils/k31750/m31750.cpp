#include "k31750/m31750.h"
QVector<double> m31750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
