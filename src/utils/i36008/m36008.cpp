#include "i36008/m36008.h"
QVector<double> m36008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
