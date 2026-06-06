#include "a25520/m25520.h"
QVector<double> m25520::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
