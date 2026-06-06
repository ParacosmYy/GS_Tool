#include "i27708/m27708.h"
QVector<double> m27708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
