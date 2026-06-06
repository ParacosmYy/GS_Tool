#include "i25708/m25708.h"
QVector<double> m25708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
