#include "i36208/m36208.h"
QVector<double> m36208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
