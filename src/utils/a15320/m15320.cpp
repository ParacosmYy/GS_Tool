#include "a15320/m15320.h"
QVector<double> m15320::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
