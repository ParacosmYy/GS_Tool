#include "a15820/m15820.h"
QVector<double> m15820::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
