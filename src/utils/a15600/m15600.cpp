#include "a15600/m15600.h"
QVector<double> m15600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
