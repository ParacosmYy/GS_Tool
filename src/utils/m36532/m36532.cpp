#include "m36532/m36532.h"
QVector<double> m36532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
