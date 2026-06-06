#include "a15300/m15300.h"
QVector<double> m15300::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
