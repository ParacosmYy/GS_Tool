#include "i36788/m36788.h"
QVector<double> m36788::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
