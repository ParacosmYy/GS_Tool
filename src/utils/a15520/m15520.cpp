#include "a15520/m15520.h"
QVector<double> m15520::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
