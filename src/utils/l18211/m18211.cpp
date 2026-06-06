#include "l18211/m18211.h"
QVector<double> m18211::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
