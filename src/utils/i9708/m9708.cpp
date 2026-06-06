#include "i9708/m9708.h"
QVector<double> m9708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
