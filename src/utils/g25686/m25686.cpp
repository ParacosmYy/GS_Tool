#include "g25686/m25686.h"
QVector<double> m25686::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
