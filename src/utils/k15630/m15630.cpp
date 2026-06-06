#include "k15630/m15630.h"
QVector<double> m15630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
