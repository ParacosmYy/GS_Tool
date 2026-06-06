#include "k17630/m17630.h"
QVector<double> m17630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
