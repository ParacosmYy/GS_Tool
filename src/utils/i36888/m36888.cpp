#include "i36888/m36888.h"
QVector<double> m36888::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
